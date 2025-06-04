static int canonicalize_ceiling_entry(struct string_list_item *item,
				      void *cb_data)
{
	int *empty_entry_found = cb_data;
	char *ceil = item->string;

	if (!*ceil) {
		*empty_entry_found = 1;
		return 0;
	} else if (!is_absolute_path(ceil)) {
		return 0;
	} else if (*empty_entry_found) {
		/* Keep entry but do not canonicalize it */
		return 1;
	} else {
		char *real_path = real_pathdup(ceil, 0);
		if (!real_path) {
			return 0;
		}
		free(item->string);
		item->string = real_path;
		return 1;
	}
}


// Source: setup.c
// Lines 1003-1026
