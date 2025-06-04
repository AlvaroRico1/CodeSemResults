void traverse_commit_list(struct rev_info *revs,
			  show_commit_fn show_commit,
			  show_object_fn show_object,
			  void *show_data)
{
	struct traversal_context ctx;
	ctx.revs = revs;
	ctx.show_commit = show_commit;
	ctx.show_object = show_object;
	ctx.show_data = show_data;
	ctx.filter = NULL;
	do_traverse(&ctx);
}


// Source: list-objects.c
// Lines 419-431
