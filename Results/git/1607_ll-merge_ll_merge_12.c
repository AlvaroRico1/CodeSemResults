int ll_merge(mmbuffer_t *result_buf,
	     const char *path,
	     mmfile_t *ancestor, const char *ancestor_label,
	     mmfile_t *ours, const char *our_label,
	     mmfile_t *theirs, const char *their_label,
	     struct index_state *istate,
	     const struct ll_merge_options *opts)
{
	struct attr_check *check = load_merge_attributes();
	static const struct ll_merge_options default_opts;
	const char *ll_driver_name = NULL;
	int marker_size = DEFAULT_CONFLICT_MARKER_SIZE;
	const struct ll_merge_driver *driver;

	if (!opts)
		opts = &default_opts;

	if (opts->renormalize) {
		normalize_file(ancestor, path, istate);
		normalize_file(ours, path, istate);
		normalize_file(theirs, path, istate);
	}

	git_check_attr(istate, path, check);
	ll_driver_name = check->items[0].value;
	if (check->items[1].value) {
		marker_size = atoi(check->items[1].value);
		if (marker_size <= 0)
			marker_size = DEFAULT_CONFLICT_MARKER_SIZE;
	}
	driver = find_ll_merge_driver(ll_driver_name);

	if (opts->virtual_ancestor) {
		if (driver->recursive)
			driver = find_ll_merge_driver(driver->recursive);
	}
	if (opts->extra_marker_size) {
		marker_size += opts->extra_marker_size;
	}
	return driver->fn(driver, result_buf, path, ancestor, ancestor_label,
			  ours, our_label, theirs, their_label,
			  opts, marker_size);
}


// Source: ll-merge.c
// Lines 365-407
