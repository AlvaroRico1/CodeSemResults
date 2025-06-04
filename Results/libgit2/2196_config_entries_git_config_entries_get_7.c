int git_config_entries_get(git_config_entry **out, git_config_entries *entries, const char *key)
{
	config_entry_map_head *entry;
	if ((entry = git_strmap_get(entries->map, key)) == NULL)
		return GIT_ENOTFOUND;
	*out = entry->entry;
	return 0;
}


// Source: config_entries.c
// Lines 169-176
