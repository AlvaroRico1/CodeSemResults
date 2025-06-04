static void assert_status_entrycount(git_repository *repo, size_t count)
{
	git_status_list *status;

	cl_git_pass(git_status_list_new(&status, repo, NULL));
	cl_assert_equal_i(count, git_status_list_entrycount(status));

	git_status_list_free(status);
}


// Source: tree.c
// Lines 12-20
