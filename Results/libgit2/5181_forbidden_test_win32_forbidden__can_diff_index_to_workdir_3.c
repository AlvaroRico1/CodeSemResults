void test_win32_forbidden__can_diff_index_to_workdir(void)
{
	git_index *index;
	git_diff *diff;
	const git_diff_delta *delta;
	git_tree *tree;
	size_t i;

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_repository_head_tree(&tree, repo));
	cl_git_pass(git_diff_index_to_workdir(&diff, repo, index, NULL));

	for (i = 0; i < git_diff_num_deltas(diff); i++) {
		delta = git_diff_get_delta(diff, i);
		cl_assert_equal_i(GIT_DELTA_DELETED, delta->status);
	}

	git_diff_free(diff);
	git_tree_free(tree);
	git_index_free(index);
}


// Source: forbidden.c
// Lines 116-136
