static int ensure_clean_index(git_repository *repo, git_index *index)
{
	git_tree *head_tree = NULL;
	git_diff *index_diff = NULL;
	int error = 0;

	if ((error = git_repository_head_tree(&head_tree, repo)) < 0 ||
		(error = git_diff_tree_to_index(
			&index_diff, repo, head_tree, index, NULL)) < 0)
		goto done;

	if (git_diff_num_deltas(index_diff) > 0) {
		git_error_set(GIT_ERROR_STASH, "%" PRIuZ " uncommitted changes exist in the index",
			git_diff_num_deltas(index_diff));
		error = GIT_EUNCOMMITTED;
	}

done:
	git_diff_free(index_diff);
	git_tree_free(head_tree);
	return error;
}


// Source: stash.c
// Lines 796-817
