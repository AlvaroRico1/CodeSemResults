int git_config_entries_get_unique(git_config_entry **out, git_config_entries *entries, const char *key)
{
	config_entry_map_head *entry;

	if ((entry = git_strmap_get(entries->map, key)) == NULL)
		return GIT_ENOTFOUND;

	if (entry->multivar) {
		git_error_set(GIT_ERROR_CONFIG, "entry is not unique due to being a multivar");
		return -1;
	}

	if (entry->entry->include_depth) {
		git_error_set(GIT_ERROR_CONFIG, "entry is not unique due to being included");
		return -1;
	}

	*out = entry->entry;

	return 0;
}


// Source: config_entries.c
// Lines 178-198
