void test_win32_forbidden__can_diff_tree_to_tree(void)
{
	git_diff *diff;
	git_tree *tree;

	cl_git_pass(git_repository_head_tree(&tree, repo));
	cl_git_pass(git_diff_tree_to_tree(&diff, repo, tree, tree, NULL));
	cl_assert_equal_i(0, git_diff_num_deltas(diff));
	git_diff_free(diff);
	git_tree_free(tree);
}


// Source: forbidden.c
// Lines 104-114
