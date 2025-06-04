int git_attr_file__new(
	git_attr_file **out,
	git_attr_file_entry *entry,
	git_attr_file_source *source)
{
	git_attr_file *attrs = git__calloc(1, sizeof(git_attr_file));
	GIT_ERROR_CHECK_ALLOC(attrs);

	if (git_mutex_init(&attrs->lock) < 0) {
		git_error_set(GIT_ERROR_OS, "failed to initialize lock");
		goto on_error;
	}

	if (git_pool_init(&attrs->pool, 1) < 0)
		goto on_error;

	GIT_REFCOUNT_INC(attrs);
	attrs->entry = entry;
	memcpy(&attrs->source, source, sizeof(git_attr_file_source));
	*out = attrs;
	return 0;

on_error:
	git__free(attrs);
	return -1;
}


// Source: attr_file.c
// Lines 33-58
