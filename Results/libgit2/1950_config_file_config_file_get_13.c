static int config_file_get(git_config_backend *cfg, const char *key, git_config_entry **out)
{
	config_file_backend *h = GIT_CONTAINER_OF(cfg, config_file_backend, parent);
	git_config_entries *entries = NULL;
	git_config_entry *entry;
	int error = 0;

	if (!h->parent.readonly && ((error = config_file_refresh(cfg)) < 0))
		return error;

	if ((error = config_file_entries_take(&entries, h)) < 0)
		return error;

	if ((error = (git_config_entries_get(&entry, entries, key))) < 0) {
		git_config_entries_free(entries);
		return error;
	}

	entry->free = config_file_entry_free;
	entry->payload = entries;
	*out = entry;

	return 0;
}


// Source: config_file.c
// Lines 344-367
