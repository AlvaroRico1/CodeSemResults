static void prepare_packed_git_mru(struct repository *r)
{
	struct packed_git *p;

	INIT_LIST_HEAD(&r->objects->packed_git_mru);

	for (p = r->objects->packed_git; p; p = p->next)
		list_add_tail(&p->mru, &r->objects->packed_git_mru);
}


// Source: packfile.c
// Lines 973-981
