int opt_parse_list_objects_filter(const struct option *opt,
				  const char *arg, int unset)
{
	struct list_objects_filter_options *filter_options = opt->value;

	if (unset || !arg)
		list_objects_filter_set_no_filter(filter_options);
	else
		parse_list_objects_filter(filter_options, arg);
	return 0;
}


// Source: list-objects-filter-options.c
// Lines 299-309
