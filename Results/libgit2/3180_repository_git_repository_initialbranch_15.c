int git_repository_initialbranch(git_str *out, git_repository *repo)
{
	git_config *config;
	git_config_entry *entry = NULL;
	const char *branch;
	int valid, error;

	if ((error = git_repository_config__weakptr(&config, repo)) < 0)
		return error;

	if ((error = git_config_get_entry(&entry, config, "init.defaultbranch")) == 0 &&
		*entry->value) {
		branch = entry->value;
	}
	else if (!error || error == GIT_ENOTFOUND) {
		branch = GIT_BRANCH_DEFAULT;
	}
	else {
		goto done;
	}

	if ((error = git_str_puts(out, GIT_REFS_HEADS_DIR)) < 0 ||
	    (error = git_str_puts(out, branch)) < 0 ||
	    (error = git_reference_name_is_valid(&valid, out->ptr)) < 0)
	    goto done;

	if (!valid) {
		git_error_set(GIT_ERROR_INVALID, "the value of init.defaultBranch is not a valid branch name");
		error = -1;
	}

done:
	git_config_entry_free(entry);
	return error;
}


// Source: repository.c
// Lines 2528-2562
