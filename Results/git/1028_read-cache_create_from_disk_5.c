static struct cache_entry *create_from_disk(struct mem_pool *ce_mem_pool,
					    unsigned int version,
					    struct ondisk_cache_entry *ondisk,
					    unsigned long *ent_size,
					    const struct cache_entry *previous_ce)
{
	struct cache_entry *ce;
	size_t len;
	const char *name;
	const unsigned hashsz = the_hash_algo->rawsz;
	const uint16_t *flagsp = (const uint16_t *)(ondisk->data + hashsz);
	unsigned int flags;
	size_t copy_len = 0;
	/*
	 * Adjacent cache entries tend to share the leading paths, so it makes
	 * sense to only store the differences in later entries.  In the v4
	 * on-disk format of the index, each on-disk cache entry stores the
	 * number of bytes to be stripped from the end of the previous name,
	 * and the bytes to append to the result, to come up with its name.
	 */
	int expand_name_field = version == 4;

	/* On-disk flags are just 16 bits */
	flags = get_be16(flagsp);
	len = flags & CE_NAMEMASK;

	if (flags & CE_EXTENDED) {
		int extended_flags;
		extended_flags = get_be16(flagsp + 1) << 16;
		/* We do not yet understand any bit out of CE_EXTENDED_FLAGS */
		if (extended_flags & ~CE_EXTENDED_FLAGS)
			die(_("unknown index entry format 0x%08x"), extended_flags);
		flags |= extended_flags;
		name = (const char *)(flagsp + 2);
	}
	else
		name = (const char *)(flagsp + 1);

	if (expand_name_field) {
		const unsigned char *cp = (const unsigned char *)name;
		size_t strip_len, previous_len;

		/* If we're at the beginning of a block, ignore the previous name */
		strip_len = decode_varint(&cp);
		if (previous_ce) {
			previous_len = previous_ce->ce_namelen;
			if (previous_len < strip_len)
				die(_("malformed name field in the index, near path '%s'"),
					previous_ce->name);
			copy_len = previous_len - strip_len;
		}
		name = (const char *)cp;
	}

	if (len == CE_NAMEMASK) {
		len = strlen(name);
		if (expand_name_field)
			len += copy_len;
	}

	ce = mem_pool__ce_alloc(ce_mem_pool, len);

	ce->ce_stat_data.sd_ctime.sec = get_be32(&ondisk->ctime.sec);
	ce->ce_stat_data.sd_mtime.sec = get_be32(&ondisk->mtime.sec);
	ce->ce_stat_data.sd_ctime.nsec = get_be32(&ondisk->ctime.nsec);
	ce->ce_stat_data.sd_mtime.nsec = get_be32(&ondisk->mtime.nsec);
	ce->ce_stat_data.sd_dev   = get_be32(&ondisk->dev);
	ce->ce_stat_data.sd_ino   = get_be32(&ondisk->ino);
	ce->ce_mode  = get_be32(&ondisk->mode);
	ce->ce_stat_data.sd_uid   = get_be32(&ondisk->uid);
	ce->ce_stat_data.sd_gid   = get_be32(&ondisk->gid);
	ce->ce_stat_data.sd_size  = get_be32(&ondisk->size);
	ce->ce_flags = flags & ~CE_NAMEMASK;
	ce->ce_namelen = len;
	ce->index = 0;
	oidread(&ce->oid, ondisk->data);
	memcpy(ce->name, name, len);
	ce->name[len] = '\0';

	if (expand_name_field) {
		if (copy_len)
			memcpy(ce->name, previous_ce->name, copy_len);
		memcpy(ce->name + copy_len, name, len + 1 - copy_len);
		*ent_size = (name - ((char *)ondisk)) + len + 1 - copy_len;
	} else {
		memcpy(ce->name, name, len + 1);
		*ent_size = ondisk_ce_size(ce);
	}
	return ce;
}


// Source: read-cache.c
// Lines 1825-1914
