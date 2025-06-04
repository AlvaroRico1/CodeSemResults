static void filter_trees_free(void *filter_data) {
	struct filter_trees_depth_data *d = filter_data;
	if (!d)
		return;
	oidmap_free(&d->seen_at_depth, 1);
	free(d);
}


// Source: list-objects-filter.c
// Lines 243-249
