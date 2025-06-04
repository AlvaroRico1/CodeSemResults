int find_pack_entry(struct repository *r, const struct object_id *oid, struct pack_entry *e)
{
	struct list_head *pos;
	struct multi_pack_index *m;

	prepare_packed_git(r);
	if (!r->objects->packed_git && !r->objects->multi_pack_index)
		return 0;

	for (m = r->objects->multi_pack_index; m; m = m->next) {
		if (fill_midx_entry(r, oid, e, m))
			return 1;
	}

	list_for_each(pos, &r->objects->packed_git_mru) {
		struct packed_git *p = list_entry(pos, struct packed_git, mru);
		if (!p->multi_pack_index && fill_pack_entry(oid, e, p)) {
			list_move(&p->mru, &r->objects->packed_git_mru);
			return 1;
		}
	}
	return 0;
}


// Source: packfile.c
// Lines 2030-2052
