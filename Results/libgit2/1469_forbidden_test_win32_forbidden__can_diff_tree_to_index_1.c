void test_win32_forbidden__can_diff_tree_to_index(void)
{
	git_diff *diff;
	git_tree *tree;

	cl_git_pass(git_repository_head_tree(&tree, repo));
	cl_git_pass(git_diff_tree_to_index(&diff, repo, tree, NULL, NULL));
	cl_assert_equal_i(0, git_diff_num_deltas(diff));
	git_diff_free(diff);
	git_tree_free(tree);
}


// Source: forbidden.c
// Lines 92-102
