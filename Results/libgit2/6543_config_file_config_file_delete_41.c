static int config_file_delete(git_config_backend *cfg, const char *name)
{
	config_file_backend *b = GIT_CONTAINER_OF(cfg, config_file_backend, parent);
	git_config_entries *entries = NULL;
	git_config_entry *entry;
	char *key = NULL;
	int error;

	if ((error = git_config__normalize_name(name, &key)) < 0)
		goto out;

	if ((error = config_file_entries_take(&entries, b)) < 0)
		goto out;

	/* Check whether we'd be modifying an included or multivar key */
	if ((error = git_config_entries_get_unique(&entry, entries, key)) < 0) {
		if (error == GIT_ENOTFOUND)
			git_error_set(GIT_ERROR_CONFIG, "could not find key '%s' to delete", name);
		goto out;
	}

	if ((error = config_file_write(b, name, entry->name, NULL, NULL)) < 0)
		goto out;

out:
	git_config_entries_free(entries);
	git__free(key);
	return error;
}


// Source: config_file.c
// Lines 396-424
