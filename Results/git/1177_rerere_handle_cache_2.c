static int handle_cache(struct index_state *istate,
			const char *path, unsigned char *hash, const char *output)
{
	mmfile_t mmfile[3] = {{NULL}};
	mmbuffer_t result = {NULL, 0};
	const struct cache_entry *ce;
	int pos, len, i, has_conflicts;
	struct rerere_io_mem io;
	int marker_size = ll_merge_marker_size(istate, path);

	/*
	 * Reproduce the conflicted merge in-core
	 */
	len = strlen(path);
	pos = index_name_pos(istate, path, len);
	if (0 <= pos)
		return -1;
	pos = -pos - 1;

	while (pos < istate->cache_nr) {
		enum object_type type;
		unsigned long size;

		ce = istate->cache[pos++];
		if (ce_namelen(ce) != len || memcmp(ce->name, path, len))
			break;
		i = ce_stage(ce) - 1;
		if (!mmfile[i].ptr) {
			mmfile[i].ptr = read_object_file(&ce->oid, &type,
							 &size);
			mmfile[i].size = size;
		}
	}


// Source: rerere.c
// Lines 939-971
