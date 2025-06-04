void close_object_store(struct raw_object_store *o)
{
	struct packed_git *p;

	for (p = o->packed_git; p; p = p->next)
		if (p->do_not_close)
			BUG("want to close pack marked 'do-not-close'");
		else
			close_pack(p);

	if (o->multi_pack_index) {
		close_midx(o->multi_pack_index);
		o->multi_pack_index = NULL;
	}

	close_commit_graph(o);
}


// Source: packfile.c
// Lines 345-361
