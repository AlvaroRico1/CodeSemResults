static int stream_list_init(
	git_writestream **out,
	git_vector *streams,
	git_filter_list *filters,
	git_writestream *target)
{
	git_writestream *last_stream = target;
	size_t i;
	int error = 0;

	*out = NULL;

	if (!filters) {
		*out = target;
		return 0;
	}

	/* Create filters last to first to get the chaining direction */
	for (i = 0; i < git_array_size(filters->filters); ++i) {
		size_t filter_idx = (filters->source.mode == GIT_FILTER_TO_WORKTREE) ?
			git_array_size(filters->filters) - 1 - i : i;

		git_filter_entry *fe = git_array_get(filters->filters, filter_idx);
		git_writestream *filter_stream;

		error = setup_stream(&filter_stream, fe, filters, last_stream);

		if (error < 0)
			goto out;

		git_vector_insert(streams, filter_stream);
		last_stream = filter_stream;
	}


// Source: filter.c
// Lines 1029-1061
