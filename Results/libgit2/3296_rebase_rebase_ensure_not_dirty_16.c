static int rebase_ensure_not_dirty(
	git_repository *repo,
	bool check_index,
	bool check_workdir,
	int fail_with)
{
	git_tree *head = NULL;
	git_index *index = NULL;
	git_diff *diff = NULL;
	int error = 0;

	if (check_index) {
		if ((error = git_repository_head_tree(&head, repo)) < 0 ||
			(error = git_repository_index(&index, repo)) < 0 ||
			(error = git_diff_tree_to_index(&diff, repo, head, index, NULL)) < 0)
			goto done;

		if (git_diff_num_deltas(diff) > 0) {
			git_error_set(GIT_ERROR_REBASE, "uncommitted changes exist in index");
			error = fail_with;
			goto done;
		}

		git_diff_free(diff);
		diff = NULL;
	}

	if (check_workdir) {
		git_diff_options diff_opts = GIT_DIFF_OPTIONS_INIT;
		diff_opts.ignore_submodules = GIT_SUBMODULE_IGNORE_UNTRACKED;
		if ((error = git_diff_index_to_workdir(&diff, repo, index, &diff_opts)) < 0)
			goto done;

		if (git_diff_num_deltas(diff) > 0) {
			git_error_set(GIT_ERROR_REBASE, "unstaged changes exist in workdir");
			error = fail_with;
			goto done;
		}
	}

done:
	git_diff_free(diff);
	git_index_free(index);
	git_tree_free(head);

	return error;
}


// Source: rebase.c
// Lines 528-574
