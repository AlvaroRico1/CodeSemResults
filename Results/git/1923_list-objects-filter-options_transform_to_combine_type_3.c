static void transform_to_combine_type(
	struct list_objects_filter_options *filter_options)
{
	assert(filter_options->choice);
	if (filter_options->choice == LOFC_COMBINE)
		return;
	{
		const int initial_sub_alloc = 2;
		struct list_objects_filter_options *sub_array =
			xcalloc(initial_sub_alloc, sizeof(*sub_array));
		sub_array[0] = *filter_options;
		memset(filter_options, 0, sizeof(*filter_options));
		filter_options->sub = sub_array;
		filter_options->sub_alloc = initial_sub_alloc;
	}


// Source: list-objects-filter-options.c
// Lines 232-246
