static void add_blame_entry(git_blame *blame, git_blame__entry *e)
{
	git_blame__entry *ent, *prev = NULL;

	origin_incref(e->suspect);

	for (ent = blame->ent; ent && ent->lno < e->lno; ent = ent->next)
		prev = ent;

	/* prev, if not NULL, is the last one that is below e */
	e->prev = prev;
	if (prev) {
		e->next = prev->next;
		prev->next = e;
	} else {
		e->next = blame->ent;
		blame->ent = e;
	}
	if (e->next)
		e->next->prev = e;
}


// Source: blame_git.c
// Lines 177-197
