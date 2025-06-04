static void find_abbrev_len_packed(struct min_abbrev_data *mad)
{
	struct multi_pack_index *m;
	struct packed_git *p;

	for (m = get_multi_pack_index(mad->repo); m; m = m->next)
		find_abbrev_len_for_midx(m, mad);
	for (p = get_packed_git(mad->repo); p; p = p->next)
		find_abbrev_len_for_pack(p, mad);
}


// Source: object-name.c
// Lines 652-661
