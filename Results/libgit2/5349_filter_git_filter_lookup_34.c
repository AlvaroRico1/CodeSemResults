git_filter *git_filter_lookup(const char *name)
{
	size_t pos;
	git_filter_def *fdef;
	git_filter *filter = NULL;

	if (git_rwlock_rdlock(&filter_registry.lock) < 0) {
		git_error_set(GIT_ERROR_OS, "failed to lock filter registry");
		return NULL;
	}

	if ((fdef = filter_registry_lookup(&pos, name)) == NULL ||
		(!fdef->initialized && filter_initialize(fdef) < 0))
		goto done;

	filter = fdef->filter;

done:
	git_rwlock_rdunlock(&filter_registry.lock);
	return filter;
}


// Source: filter.c
// Lines 348-368
