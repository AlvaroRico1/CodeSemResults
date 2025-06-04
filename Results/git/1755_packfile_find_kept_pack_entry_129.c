int find_kept_pack_entry(struct repository *r,
			 const struct object_id *oid,
			 unsigned flags,
			 struct pack_entry *e)
{
	struct packed_git **cache;

	for (cache = kept_pack_cache(r, flags); *cache; cache++) {
		struct packed_git *p = *cache;
		if (fill_pack_entry(oid, e, p))
			return 1;
	}


// Source: packfile.c
// Lines 2099-2110
