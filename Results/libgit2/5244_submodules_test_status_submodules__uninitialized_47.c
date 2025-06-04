void test_status_submodules__uninitialized(void)
{
	git_repository *cloned_repo;
	git_status_list *statuslist;

	g_repo = cl_git_sandbox_init("submod2");

	cl_git_pass(git_clone(&cloned_repo, "submod2", "submod2-clone", NULL));

	cl_git_pass(git_status_list_new(&statuslist, cloned_repo, NULL));
	cl_assert_equal_i(0, git_status_list_entrycount(statuslist));

	git_status_list_free(statuslist);
	git_repository_free(cloned_repo);
	cl_git_sandbox_cleanup();
}


// Source: submodules.c
// Lines 218-233
