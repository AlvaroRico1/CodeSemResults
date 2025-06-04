static int write_midx_oid_fanout(struct hashfile *f,
				 void *data)
{
	struct write_midx_context *ctx = data;
	struct pack_midx_entry *list = ctx->entries;
	struct pack_midx_entry *last = ctx->entries + ctx->entries_nr;
	uint32_t count = 0;
	uint32_t i;

	/*
	* Write the first-level table (the list is sorted,
	* but we use a 256-entry lookup to be able to avoid
	* having to do eight extra binary search iterations).
	*/
	for (i = 0; i < 256; i++) {
		struct pack_midx_entry *next = list;

		while (next < last && next->oid.hash[0] == i) {
			count++;
			next++;
		}

		hashwrite_be32(f, count);
		list = next;
	}

	return 0;
}


// Source: midx.c
// Lines 700-727
