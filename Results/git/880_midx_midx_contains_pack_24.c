int midx_contains_pack(struct multi_pack_index *m, const char *idx_or_pack_name)
{
	uint32_t first = 0, last = m->num_packs;

	while (first < last) {
		uint32_t mid = first + (last - first) / 2;
		const char *current;
		int cmp;

		current = m->pack_names[mid];
		cmp = cmp_idx_or_pack_name(idx_or_pack_name, current);
		if (!cmp)
			return 1;
		if (cmp > 0) {
			first = mid + 1;
			continue;
		}
		last = mid;
	}


// Source: midx.c
// Lines 359-377
