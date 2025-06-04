int prepare_multi_pack_index_one(struct repository *r, const char *object_dir, int local)
{
	struct multi_pack_index *m;
	struct multi_pack_index *m_search;

	prepare_repo_settings(r);
	if (!r->settings.core_multi_pack_index)
		return 0;

	for (m_search = r->objects->multi_pack_index; m_search; m_search = m_search->next)
		if (!strcmp(object_dir, m_search->object_dir))
			return 1;

	m = load_multi_pack_index(object_dir, local);

	if (m) {
		struct multi_pack_index *mp = r->objects->multi_pack_index;
		if (mp) {
			m->next = mp->next;
			mp->next = m;
		} else
			r->objects->multi_pack_index = m;
		return 1;
	}

	return 0;
}


// Source: midx.c
// Lines 382-408
