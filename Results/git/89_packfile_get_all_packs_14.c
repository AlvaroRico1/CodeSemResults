struct packed_git *get_all_packs(struct repository *r)
{
	struct multi_pack_index *m;

	prepare_packed_git(r);
	for (m = r->objects->multi_pack_index; m; m = m->next) {
		uint32_t i;
		for (i = 0; i < m->num_packs; i++)
			prepare_midx_pack(r, m, i);
	}

	return r->objects->packed_git;
}


// Source: packfile.c
// Lines 1039-1051
