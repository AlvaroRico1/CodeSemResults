static int config_file_set_entries(git_config_backend *cfg, git_config_entries *entries)
{
	config_file_backend *b = GIT_CONTAINER_OF(cfg, config_file_backend, parent);
	git_config_entries *old = NULL;
	int error;

	if (b->parent.readonly) {
		git_error_set(GIT_ERROR_CONFIG, "this backend is read-only");
		return -1;
	}

	if ((error = git_mutex_lock(&b->values_mutex)) < 0) {
		git_error_set(GIT_ERROR_OS, "failed to lock config backend");
		goto out;
	}

	old = b->entries;
	b->entries = entries;

	git_mutex_unlock(&b->values_mutex);

out:
	git_config_entries_free(old);
	return error;
}


// Source: config_file.c
// Lines 178-202
