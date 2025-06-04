robj *rdbLoadObject(int rdbtype, rio *rdb, sds key, int *error) {
    robj *o = NULL, *ele, *dec;
    uint64_t len;
    unsigned int i;

    /* Set default error of load object, it will be set to 0 on success. */
    if (error) *error = RDB_LOAD_ERR_OTHER;

    int deep_integrity_validation = server.sanitize_dump_payload == SANITIZE_DUMP_YES;
    if (server.sanitize_dump_payload == SANITIZE_DUMP_CLIENTS) {
        /* Skip sanitization when loading (an RDB), or getting a RESTORE command
         * from either the master or a client using an ACL user with the skip-sanitize-payload flag. */
        int skip = server.loading ||
            (server.current_client && (server.current_client->flags & CLIENT_MASTER));
        if (!skip && server.current_client && server.current_client->user)
            skip = !!(server.current_client->user->flags & USER_FLAG_SANITIZE_PAYLOAD_SKIP);
        deep_integrity_validation = !skip;
    }

    if (rdbtype == RDB_TYPE_STRING) {
        /* Read string value */
        if ((o = rdbLoadEncodedStringObject(rdb)) == NULL) return NULL;
        o = tryObjectEncoding(o);
    } else if (rdbtype == RDB_TYPE_LIST) {
        /* Read list value */
        if ((len = rdbLoadLen(rdb,NULL)) == RDB_LENERR) return NULL;
        if (len == 0) goto emptykey;

        o = createQuicklistObject();
        quicklistSetOptions(o->ptr, server.list_max_ziplist_size,
                            server.list_compress_depth);

        /* Load every single element of the list */
        while(len--) {
            if ((ele = rdbLoadEncodedStringObject(rdb)) == NULL) {
                decrRefCount(o);
                return NULL;
            }
            dec = getDecodedObject(ele);
            size_t len = sdslen(dec->ptr);
            quicklistPushTail(o->ptr, dec->ptr, len);
            decrRefCount(dec);
            decrRefCount(ele);
        }
    } else if (rdbtype == RDB_TYPE_SET) {
        /* Read Set value */
        if ((len = rdbLoadLen(rdb,NULL)) == RDB_LENERR) return NULL;
        if (len == 0) goto emptykey;

        /* Use a regular set when there are too many entries. */
        size_t max_entries = server.set_max_intset_entries;
        if (max_entries >= 1<<30) max_entries = 1<<30;
        if (len > max_entries) {
            o = createSetObject();
            /* It's faster to expand the dict to the right size asap in order
             * to avoid rehashing */
            if (len > DICT_HT_INITIAL_SIZE && dictTryExpand(o->ptr,len) != DICT_OK) {
                rdbReportCorruptRDB("OOM in dictTryExpand %llu", (unsigned long long)len);
                decrRefCount(o);
                return NULL;
            }
        } else {
            o = createIntsetObject();
        }

        /* Load every single element of the set */
        for (i = 0; i < len; i++) {
            long long llval;
            sds sdsele;

            if ((sdsele = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                decrRefCount(o);
                return NULL;
            }

            if (o->encoding == OBJ_ENCODING_INTSET) {
                /* Fetch integer value from element. */
                if (isSdsRepresentableAsLongLong(sdsele,&llval) == C_OK) {
                    uint8_t success;
                    o->ptr = intsetAdd(o->ptr,llval,&success);
                    if (!success) {
                        rdbReportCorruptRDB("Duplicate set members detected");
                        decrRefCount(o);
                        sdsfree(sdsele);
                        return NULL;
                    }
                } else {
                    setTypeConvert(o,OBJ_ENCODING_HT);
                    if (dictTryExpand(o->ptr,len) != DICT_OK) {
                        rdbReportCorruptRDB("OOM in dictTryExpand %llu", (unsigned long long)len);
                        sdsfree(sdsele);
                        decrRefCount(o);
                        return NULL;
                    }
                }
            }

            /* This will also be called when the set was just converted
             * to a regular hash table encoded set. */
            if (o->encoding == OBJ_ENCODING_HT) {
                if (dictAdd((dict*)o->ptr,sdsele,NULL) != DICT_OK) {
                    rdbReportCorruptRDB("Duplicate set members detected");
                    decrRefCount(o);
                    sdsfree(sdsele);
                    return NULL;
                }
            } else {
                sdsfree(sdsele);
            }
        }
    } else if (rdbtype == RDB_TYPE_ZSET_2 || rdbtype == RDB_TYPE_ZSET) {
        /* Read list/set value. */
        uint64_t zsetlen;
        size_t maxelelen = 0, totelelen = 0;
        zset *zs;

        if ((zsetlen = rdbLoadLen(rdb,NULL)) == RDB_LENERR) return NULL;
        if (zsetlen == 0) goto emptykey;

        o = createZsetObject();
        zs = o->ptr;

        if (zsetlen > DICT_HT_INITIAL_SIZE && dictTryExpand(zs->dict,zsetlen) != DICT_OK) {
            rdbReportCorruptRDB("OOM in dictTryExpand %llu", (unsigned long long)zsetlen);
            decrRefCount(o);
            return NULL;
        }

        /* Load every single element of the sorted set. */
        while(zsetlen--) {
            sds sdsele;
            double score;
            zskiplistNode *znode;

            if ((sdsele = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                decrRefCount(o);
                return NULL;
            }

            if (rdbtype == RDB_TYPE_ZSET_2) {
                if (rdbLoadBinaryDoubleValue(rdb,&score) == -1) {
                    decrRefCount(o);
                    sdsfree(sdsele);
                    return NULL;
                }
            } else {
                if (rdbLoadDoubleValue(rdb,&score) == -1) {
                    decrRefCount(o);
                    sdsfree(sdsele);
                    return NULL;
                }
            }

            /* Don't care about integer-encoded strings. */
            if (sdslen(sdsele) > maxelelen) maxelelen = sdslen(sdsele);
            totelelen += sdslen(sdsele);

            znode = zslInsert(zs->zsl,score,sdsele);
            if (dictAdd(zs->dict,sdsele,&znode->score) != DICT_OK) {
                rdbReportCorruptRDB("Duplicate zset fields detected");
                decrRefCount(o);
                /* no need to free 'sdsele', will be released by zslFree together with 'o' */
                return NULL;
            }
        }

        /* Convert *after* loading, since sorted sets are not stored ordered. */
        if (zsetLength(o) <= server.zset_max_ziplist_entries &&
            maxelelen <= server.zset_max_ziplist_value &&
            ziplistSafeToAdd(NULL, totelelen))
        {
            zsetConvert(o,OBJ_ENCODING_ZIPLIST);
        }
    } else if (rdbtype == RDB_TYPE_HASH) {
        uint64_t len;
        int ret;
        sds field, value;
        dict *dupSearchDict = NULL;

        len = rdbLoadLen(rdb, NULL);
        if (len == RDB_LENERR) return NULL;
        if (len == 0) goto emptykey;

        o = createHashObject();

        /* Too many entries? Use a hash table right from the start. */
        if (len > server.hash_max_ziplist_entries)
            hashTypeConvert(o, OBJ_ENCODING_HT);
        else if (deep_integrity_validation) {
            /* In this mode, we need to guarantee that the server won't crash
             * later when the ziplist is converted to a dict.
             * Create a set (dict with no values) to for a dup search.
             * We can dismiss it as soon as we convert the ziplist to a hash. */
            dupSearchDict = dictCreate(&hashDictType, NULL);
        }


        /* Load every field and value into the ziplist */
        while (o->encoding == OBJ_ENCODING_ZIPLIST && len > 0) {
            len--;
            /* Load raw strings */
            if ((field = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                decrRefCount(o);
                if (dupSearchDict) dictRelease(dupSearchDict);
                return NULL;
            }
            if ((value = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                sdsfree(field);
                decrRefCount(o);
                if (dupSearchDict) dictRelease(dupSearchDict);
                return NULL;
            }

            if (dupSearchDict) {
                sds field_dup = sdsdup(field);
                if (dictAdd(dupSearchDict, field_dup, NULL) != DICT_OK) {
                    rdbReportCorruptRDB("Hash with dup elements");
                    dictRelease(dupSearchDict);
                    decrRefCount(o);
                    sdsfree(field_dup);
                    sdsfree(field);
                    sdsfree(value);
                    return NULL;
                }
            }

            /* Convert to hash table if size threshold is exceeded */
            if (sdslen(field) > server.hash_max_ziplist_value ||
                sdslen(value) > server.hash_max_ziplist_value ||
                !ziplistSafeToAdd(o->ptr, sdslen(field)+sdslen(value)))
            {
                hashTypeConvert(o, OBJ_ENCODING_HT);
                ret = dictAdd((dict*)o->ptr, field, value);
                if (ret == DICT_ERR) {
                    rdbReportCorruptRDB("Duplicate hash fields detected");
                    if (dupSearchDict) dictRelease(dupSearchDict);
                    sdsfree(value);
                    sdsfree(field);
                    decrRefCount(o);
                    return NULL;
                }
                break;
            }

            /* Add pair to ziplist */
            o->ptr = ziplistPush(o->ptr, (unsigned char*)field,
                    sdslen(field), ZIPLIST_TAIL);
            o->ptr = ziplistPush(o->ptr, (unsigned char*)value,
                    sdslen(value), ZIPLIST_TAIL);

            sdsfree(field);
            sdsfree(value);
        }

        if (dupSearchDict) {
            /* We no longer need this, from now on the entries are added
             * to a dict so the check is performed implicitly. */
            dictRelease(dupSearchDict);
            dupSearchDict = NULL;
        }

        if (o->encoding == OBJ_ENCODING_HT && len > DICT_HT_INITIAL_SIZE) {
            if (dictTryExpand(o->ptr,len) != DICT_OK) {
                rdbReportCorruptRDB("OOM in dictTryExpand %llu", (unsigned long long)len);
                decrRefCount(o);
                return NULL;
            }
        }

        /* Load remaining fields and values into the hash table */
        while (o->encoding == OBJ_ENCODING_HT && len > 0) {
            len--;
            /* Load encoded strings */
            if ((field = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                decrRefCount(o);
                return NULL;
            }
            if ((value = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL)) == NULL) {
                sdsfree(field);
                decrRefCount(o);
                return NULL;
            }

            /* Add pair to hash table */
            ret = dictAdd((dict*)o->ptr, field, value);
            if (ret == DICT_ERR) {
                rdbReportCorruptRDB("Duplicate hash fields detected");
                sdsfree(value);
                sdsfree(field);
                decrRefCount(o);
                return NULL;
            }
        }

        /* All pairs should be read by now */
        serverAssert(len == 0);
    } else if (rdbtype == RDB_TYPE_LIST_QUICKLIST) {
        if ((len = rdbLoadLen(rdb,NULL)) == RDB_LENERR) return NULL;
        if (len == 0) goto emptykey;

        o = createQuicklistObject();
        quicklistSetOptions(o->ptr, server.list_max_ziplist_size,
                            server.list_compress_depth);

        while (len--) {
            size_t encoded_len;
            unsigned char *zl =
                rdbGenericLoadStringObject(rdb,RDB_LOAD_PLAIN,&encoded_len);
            if (zl == NULL) {
                decrRefCount(o);
                return NULL;
            }
            if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
            if (!ziplistValidateIntegrity(zl, encoded_len, deep_integrity_validation, NULL, NULL)) {
                rdbReportCorruptRDB("Ziplist integrity check failed.");
                decrRefCount(o);
                zfree(zl);
                return NULL;
            }

            /* Silently skip empty ziplists, if we'll end up with empty quicklist we'll fail later. */
            if (ziplistLen(zl) == 0) {
                zfree(zl);
                continue;
            } else {
                quicklistAppendZiplist(o->ptr, zl);
            }
        }

        if (quicklistCount(o->ptr) == 0) {
            decrRefCount(o);
            goto emptykey;
        }
    } else if (rdbtype == RDB_TYPE_HASH_ZIPMAP  ||
               rdbtype == RDB_TYPE_LIST_ZIPLIST ||
               rdbtype == RDB_TYPE_SET_INTSET   ||
               rdbtype == RDB_TYPE_ZSET_ZIPLIST ||
               rdbtype == RDB_TYPE_HASH_ZIPLIST)
    {
        size_t encoded_len;
        unsigned char *encoded =
            rdbGenericLoadStringObject(rdb,RDB_LOAD_PLAIN,&encoded_len);
        if (encoded == NULL) return NULL;

        o = createObject(OBJ_STRING,encoded); /* Obj type fixed below. */

        /* Fix the object encoding, and make sure to convert the encoded
         * data type into the base type if accordingly to the current
         * configuration there are too many elements in the encoded data
         * type. Note that we only check the length and not max element
         * size as this is an O(N) scan. Eventually everything will get
         * converted. */
        switch(rdbtype) {
            case RDB_TYPE_HASH_ZIPMAP:
                /* Since we don't keep zipmaps anymore, the rdb loading for these
                 * is O(n) anyway, use `deep` validation. */
                if (!zipmapValidateIntegrity(encoded, encoded_len, 1)) {
                    rdbReportCorruptRDB("Zipmap integrity check failed.");
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    return NULL;
                }
                /* Convert to ziplist encoded hash. This must be deprecated
                 * when loading dumps created by Redis 2.4 gets deprecated. */
                {
                    unsigned char *zl = ziplistNew();
                    unsigned char *zi = zipmapRewind(o->ptr);
                    unsigned char *fstr, *vstr;
                    unsigned int flen, vlen;
                    unsigned int maxlen = 0;
                    dict *dupSearchDict = dictCreate(&hashDictType, NULL);

                    while ((zi = zipmapNext(zi, &fstr, &flen, &vstr, &vlen)) != NULL) {
                        if (flen > maxlen) maxlen = flen;
                        if (vlen > maxlen) maxlen = vlen;

                        /* search for duplicate records */
                        sds field = sdstrynewlen(fstr, flen);
                        if (!field || dictAdd(dupSearchDict, field, NULL) != DICT_OK ||
                            !ziplistSafeToAdd(zl, (size_t)flen + vlen)) {
                            rdbReportCorruptRDB("Hash zipmap with dup elements, or big length (%u)", flen);
                            dictRelease(dupSearchDict);
                            sdsfree(field);
                            zfree(encoded);
                            o->ptr = NULL;
                            decrRefCount(o);
                            return NULL;
                        }

                        zl = ziplistPush(zl, fstr, flen, ZIPLIST_TAIL);
                        zl = ziplistPush(zl, vstr, vlen, ZIPLIST_TAIL);
                    }

                    dictRelease(dupSearchDict);
                    zfree(o->ptr);
                    o->ptr = zl;
                    o->type = OBJ_HASH;
                    o->encoding = OBJ_ENCODING_ZIPLIST;

                    if (hashTypeLength(o) > server.hash_max_ziplist_entries ||
                        maxlen > server.hash_max_ziplist_value)
                    {
                        hashTypeConvert(o, OBJ_ENCODING_HT);
                    }
                }
                break;
            case RDB_TYPE_LIST_ZIPLIST:
                if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
                if (!ziplistValidateIntegrity(encoded, encoded_len, deep_integrity_validation, NULL, NULL)) {
                    rdbReportCorruptRDB("List ziplist integrity check failed.");
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    return NULL;
                }

                if (ziplistLen(encoded) == 0) {
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    goto emptykey;
                }

                o->type = OBJ_LIST;
                o->encoding = OBJ_ENCODING_ZIPLIST;
                listTypeConvert(o,OBJ_ENCODING_QUICKLIST);
                break;
            case RDB_TYPE_SET_INTSET:
                if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
                if (!intsetValidateIntegrity(encoded, encoded_len, deep_integrity_validation)) {
                    rdbReportCorruptRDB("Intset integrity check failed.");
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    return NULL;
                }
                o->type = OBJ_SET;
                o->encoding = OBJ_ENCODING_INTSET;
                if (intsetLen(o->ptr) > server.set_max_intset_entries)
                    setTypeConvert(o,OBJ_ENCODING_HT);
                break;
            case RDB_TYPE_ZSET_ZIPLIST:
                if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
                if (!zsetZiplistValidateIntegrity(encoded, encoded_len, deep_integrity_validation)) {
                    rdbReportCorruptRDB("Zset ziplist integrity check failed.");
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    return NULL;
                }
                o->type = OBJ_ZSET;
                o->encoding = OBJ_ENCODING_ZIPLIST;
                if (zsetLength(o) == 0) {
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    goto emptykey;
                }

                if (zsetLength(o) > server.zset_max_ziplist_entries)
                    zsetConvert(o,OBJ_ENCODING_SKIPLIST);
                break;
            case RDB_TYPE_HASH_ZIPLIST:
                if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
                if (!hashZiplistValidateIntegrity(encoded, encoded_len, deep_integrity_validation)) {
                    rdbReportCorruptRDB("Hash ziplist integrity check failed.");
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    return NULL;
                }
                o->type = OBJ_HASH;
                o->encoding = OBJ_ENCODING_ZIPLIST;
                if (hashTypeLength(o) == 0) {
                    zfree(encoded);
                    o->ptr = NULL;
                    decrRefCount(o);
                    goto emptykey;
                }

                if (hashTypeLength(o) > server.hash_max_ziplist_entries)
                    hashTypeConvert(o, OBJ_ENCODING_HT);
                break;
            default:
                /* totally unreachable */
                rdbReportCorruptRDB("Unknown RDB encoding type %d",rdbtype);
                break;
        }
    } else if (rdbtype == RDB_TYPE_STREAM_LISTPACKS) {
        o = createStreamObject();
        stream *s = o->ptr;
        uint64_t listpacks = rdbLoadLen(rdb,NULL);
        if (listpacks == RDB_LENERR) {
            rdbReportReadError("Stream listpacks len loading failed.");
            decrRefCount(o);
            return NULL;
        }

        while(listpacks--) {
            /* Get the master ID, the one we'll use as key of the radix tree
             * node: the entries inside the listpack itself are delta-encoded
             * relatively to this ID. */
            sds nodekey = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL);
            if (nodekey == NULL) {
                rdbReportReadError("Stream master ID loading failed: invalid encoding or I/O error.");
                decrRefCount(o);
                return NULL;
            }
            if (sdslen(nodekey) != sizeof(streamID)) {
                rdbReportCorruptRDB("Stream node key entry is not the "
                                        "size of a stream ID");
                sdsfree(nodekey);
                decrRefCount(o);
                return NULL;
            }

            /* Load the listpack. */
            size_t lp_size;
            unsigned char *lp =
                rdbGenericLoadStringObject(rdb,RDB_LOAD_PLAIN,&lp_size);
            if (lp == NULL) {
                rdbReportReadError("Stream listpacks loading failed.");
                sdsfree(nodekey);
                decrRefCount(o);
                return NULL;
            }
            if (deep_integrity_validation) server.stat_dump_payload_sanitizations++;
            if (!streamValidateListpackIntegrity(lp, lp_size, deep_integrity_validation)) {
                rdbReportCorruptRDB("Stream listpack integrity check failed.");
                sdsfree(nodekey);
                decrRefCount(o);
                zfree(lp);
                return NULL;
            }

            unsigned char *first = lpFirst(lp);
            if (first == NULL) {
                /* Serialized listpacks should never be empty, since on
                 * deletion we should remove the radix tree key if the
                 * resulting listpack is empty. */
                rdbReportCorruptRDB("Empty listpack inside stream");
                sdsfree(nodekey);
                decrRefCount(o);
                zfree(lp);
                return NULL;
            }

            /* Insert the key in the radix tree. */
            int retval = raxTryInsert(s->rax,
                (unsigned char*)nodekey,sizeof(streamID),lp,NULL);
            sdsfree(nodekey);
            if (!retval) {
                rdbReportCorruptRDB("Listpack re-added with existing key");
                decrRefCount(o);
                zfree(lp);
                return NULL;
            }
        }
        /* Load total number of items inside the stream. */
        s->length = rdbLoadLen(rdb,NULL);

        /* Load the last entry ID. */
        s->last_id.ms = rdbLoadLen(rdb,NULL);
        s->last_id.seq = rdbLoadLen(rdb,NULL);

        if (rioGetReadError(rdb)) {
            rdbReportReadError("Stream object metadata loading failed.");
            decrRefCount(o);
            return NULL;
        }

        /* Consumer groups loading */
        uint64_t cgroups_count = rdbLoadLen(rdb,NULL);
        if (cgroups_count == RDB_LENERR) {
            rdbReportReadError("Stream cgroup count loading failed.");
            decrRefCount(o);
            return NULL;
        }
        while(cgroups_count--) {
            /* Get the consumer group name and ID. We can then create the
             * consumer group ASAP and populate its structure as
             * we read more data. */
            streamID cg_id;
            sds cgname = rdbGenericLoadStringObject(rdb,RDB_LOAD_SDS,NULL);
            if (cgname == NULL) {
                rdbReportReadError(
                    "Error reading the consumer group name from Stream");
                decrRefCount(o);
                return NULL;
            }

            cg_id.ms = rdbLoadLen(rdb,NULL);
            cg_id.seq = rdbLoadLen(rdb,NULL);
            if (rioGetReadError(rdb)) {
                rdbReportReadError("Stream cgroup ID loading failed.");
                sdsfree(cgname);
                decrRefCount(o);
                return NULL;
            }

            streamCG *cgroup = streamCreateCG(s,cgname,sdslen(cgname),&cg_id);
            if (cgroup == NULL) {
                rdbReportCorruptRDB("Duplicated consumer group name %s",
                                         cgname);
                decrRefCount(o);
                sdsfree(cgname);
                return NULL;
            }


// Source: rdb.c
// Lines 1526-2134
