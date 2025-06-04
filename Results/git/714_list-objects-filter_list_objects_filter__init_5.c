struct filter *list_objects_filter__init(
	struct oidset *omitted,
	struct list_objects_filter_options *filter_options)
{
	struct filter *filter;
	filter_init_fn init_fn;

	assert((sizeof(s_filters) / sizeof(s_filters[0])) == LOFC__COUNT);

	if (!filter_options)
		return NULL;

	if (filter_options->choice >= LOFC__COUNT)
		BUG("invalid list-objects filter choice: %d",
		    filter_options->choice);

	init_fn = s_filters[filter_options->choice];
	if (!init_fn)
		return NULL;

	CALLOC_ARRAY(filter, 1);
	filter->omits = omitted;
	init_fn(filter_options, filter);
	return filter;
}


// Source: list-objects-filter.c
// Lines 773-797
