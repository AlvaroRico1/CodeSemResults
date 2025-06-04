int check_object_signature(struct repository *r, const struct object_id *oid,
			   void *map, unsigned long size, const char *type)
{
	struct object_id real_oid;
	enum object_type obj_type;
	struct git_istream *st;
	git_hash_ctx c;
	char hdr[MAX_HEADER_LEN];
	int hdrlen;

	if (map) {
		hash_object_file(r->hash_algo, map, size, type, &real_oid);
		return !oideq(oid, &real_oid) ? -1 : 0;
	}

	st = open_istream(r, oid, &obj_type, &size, NULL);
	if (!st)
		return -1;

	/* Generate the header */
	hdrlen = xsnprintf(hdr, sizeof(hdr), "%s %"PRIuMAX , type_name(obj_type), (uintmax_t)size) + 1;

	/* Sha1.. */
	r->hash_algo->init_fn(&c);
	r->hash_algo->update_fn(&c, hdr, hdrlen);
	for (;;) {
		char buf[1024 * 16];
		ssize_t readlen = read_istream(st, buf, sizeof(buf));

		if (readlen < 0) {
			close_istream(st);
			return -1;
		}
		if (!readlen)
			break;
		r->hash_algo->update_fn(&c, buf, readlen);
	}
	r->hash_algo->final_oid_fn(&real_oid, &c);
	close_istream(st);
	return !oideq(oid, &real_oid) ? -1 : 0;
}


// Source: object-file.c
// Lines 1018-1058
