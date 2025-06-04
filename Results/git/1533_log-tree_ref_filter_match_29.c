static int ref_filter_match(const char *refname,
			    const struct decoration_filter *filter)
{
	struct string_list_item *item;
	const struct string_list *exclude_patterns = filter->exclude_ref_pattern;
	const struct string_list *include_patterns = filter->include_ref_pattern;
	const struct string_list *exclude_patterns_config =
				filter->exclude_ref_config_pattern;

	if (exclude_patterns && exclude_patterns->nr) {
		for_each_string_list_item(item, exclude_patterns) {
			if (match_ref_pattern(refname, item))
				return 0;
		}
	}

	if (include_patterns && include_patterns->nr) {
		for_each_string_list_item(item, include_patterns) {
			if (match_ref_pattern(refname, item))
				return 1;
		}
		return 0;
	}

	if (exclude_patterns_config && exclude_patterns_config->nr) {
		for_each_string_list_item(item, exclude_patterns_config) {
			if (match_ref_pattern(refname, item))
				return 0;
		}
	}

	return 1;
}


// Source: log-tree.c
// Lines 99-131
