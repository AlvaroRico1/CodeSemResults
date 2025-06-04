unsigned long repo_approximate_object_count(struct repository *r)
{
	if (!r->objects->approximate_object_count_valid) {
		unsigned long count;
		struct multi_pack_index *m;
		struct packed_git *p;

		prepare_packed_git(r);
		count = 0;
		for (m = get_multi_pack_index(r); m; m = m->next)
			count += m->num_objects;
		for (p = r->objects->packed_git; p; p = p->next) {
			if (open_pack_index(p))
				continue;
			count += p->num_objects;
		}
		r->objects->approximate_object_count = count;
		r->objects->approximate_object_count_valid = 1;
	}


// Source: packfile.c
// Lines 906-924
