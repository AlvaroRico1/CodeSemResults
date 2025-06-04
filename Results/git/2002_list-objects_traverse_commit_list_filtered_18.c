void traverse_commit_list_filtered(
	struct list_objects_filter_options *filter_options,
	struct rev_info *revs,
	show_commit_fn show_commit,
	show_object_fn show_object,
	void *show_data,
	struct oidset *omitted)
{
	struct traversal_context ctx;

	ctx.revs = revs;
	ctx.show_object = show_object;
	ctx.show_commit = show_commit;
	ctx.show_data = show_data;
	ctx.filter = list_objects_filter__init(omitted, filter_options);
	do_traverse(&ctx);
	list_objects_filter__free(ctx.filter);
}


// Source: list-objects.c
// Lines 433-450
