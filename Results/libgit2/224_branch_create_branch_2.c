static int create_branch(
	git_reference **ref_out,
	git_repository *repository,
	const char *branch_name,
	const git_commit *commit,
	const char *from,
	int force)
{
	int is_unmovable_head = 0;
	git_reference *branch = NULL;
	git_str canonical_branch_name = GIT_STR_INIT,
			  log_message = GIT_STR_INIT;
	int error = -1;
	int bare = git_repository_is_bare(repository);

	GIT_ASSERT_ARG(branch_name);
	GIT_ASSERT_ARG(commit);
	GIT_ASSERT_ARG(ref_out);
	GIT_ASSERT_ARG(git_commit_owner(commit) == repository);

	if (!git__strcmp(branch_name, "HEAD")) {
		git_error_set(GIT_ERROR_REFERENCE, "'HEAD' is not a valid branch name");
		error = -1;
		goto cleanup;
	}

	if (force && !bare && git_branch_lookup(&branch, repository, branch_name, GIT_BRANCH_LOCAL) == 0) {
		error = git_branch_is_head(branch);
		git_reference_free(branch);
		branch = NULL;

		if (error < 0)
			goto cleanup;

		is_unmovable_head = error;
	}

	if (is_unmovable_head && force) {
		git_error_set(GIT_ERROR_REFERENCE, "cannot force update branch '%s' as it is "
			"the current HEAD of the repository.", branch_name);
		error = -1;
		goto cleanup;
	}

	if (git_str_joinpath(&canonical_branch_name, GIT_REFS_HEADS_DIR, branch_name) < 0)
		goto cleanup;

	if (git_str_printf(&log_message, "branch: Created from %s", from) < 0)
		goto cleanup;

	error = git_reference_create(&branch, repository,
		git_str_cstr(&canonical_branch_name), git_commit_id(commit), force,
		git_str_cstr(&log_message));

	if (!error)
		*ref_out = branch;

cleanup:
	git_str_dispose(&canonical_branch_name);
	git_str_dispose(&log_message);
	return error;
}


// Source: branch.c
// Lines 56-117
