static enum list_objects_filter_result filter_combine(
	struct repository *r,
	enum list_objects_filter_situation filter_situation,
	struct object *obj,
	const char *pathname,
	const char *filename,
	struct oidset *omits,
	void *filter_data)
{
	struct combine_filter_data *d = filter_data;
	enum list_objects_filter_result combined_result =
		LOFR_DO_SHOW | LOFR_MARK_SEEN | LOFR_SKIP_TREE;
	size_t sub;

	for (sub = 0; sub < d->nr; sub++) {
		enum list_objects_filter_result sub_result = process_subfilter(
			r, filter_situation, obj, pathname, filename,
			&d->sub[sub]);
		if (!(sub_result & LOFR_DO_SHOW))
			combined_result &= ~LOFR_DO_SHOW;
		if (!(sub_result & LOFR_MARK_SEEN))
			combined_result &= ~LOFR_MARK_SEEN;
		if (!d->sub[sub].is_skipping_tree)
			combined_result &= ~LOFR_SKIP_TREE;
	}

	return combined_result;
}


// Source: list-objects-filter.c
// Lines 672-699
