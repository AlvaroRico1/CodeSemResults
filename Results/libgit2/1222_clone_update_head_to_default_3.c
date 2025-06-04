static int update_head_to_default(git_repository *repo)
{
	git_str initialbranch = GIT_STR_INIT;
	const char *branch_name;
	int error = 0;

	if ((error = git_repository_initialbranch(&initialbranch, repo)) < 0)
		goto done;

	if (git__prefixcmp(initialbranch.ptr, GIT_REFS_HEADS_DIR) != 0) {
		git_error_set(GIT_ERROR_INVALID, "invalid initial branch '%s'", initialbranch.ptr);
		error = -1;
		goto done;
	}

	branch_name = initialbranch.ptr + strlen(GIT_REFS_HEADS_DIR);

	error = setup_tracking_config(repo, branch_name, GIT_REMOTE_ORIGIN,
		initialbranch.ptr);

done:
	git_str_dispose(&initialbranch);
	return error;
}


// Source: clone.c
// Lines 140-163
