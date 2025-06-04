int git_config_entries_new(git_config_entries **out)
{
	git_config_entries *entries;
	int error;

	entries = git__calloc(1, sizeof(git_config_entries));
	GIT_ERROR_CHECK_ALLOC(entries);
	GIT_REFCOUNT_INC(entries);

	if ((error = git_strmap_new(&entries->map)) < 0)
		git__free(entries);
	else
		*out = entries;

	return error;
}


// Source: config_entries.c
// Lines 33-48
