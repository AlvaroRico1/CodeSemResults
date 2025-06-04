void test_status_worktree_init__status_file_without_index_or_workdir(void)
{
	git_repository *repo;
	unsigned int status = 0;
	git_index *index;

	cl_git_pass(p_mkdir("wd", 0777));

	cl_git_pass(git_repository_open(&repo, cl_fixture("testrepo.git")));
	cl_git_pass(git_repository_set_workdir(repo, "wd", false));

	cl_git_pass(git_index_open(&index, "empty-index"));
	cl_assert_equal_i(0, (int)git_index_entrycount(index));
	git_repository_set_index(repo, index);

	cl_git_pass(git_status_file(&status, repo, "branch_file.txt"));

	cl_assert_equal_i(GIT_STATUS_INDEX_DELETED, status);

	git_repository_free(repo);
	git_index_free(index);
	cl_git_pass(p_rmdir("wd"));
}


// Source: worktree_init.c
// Lines 57-79
