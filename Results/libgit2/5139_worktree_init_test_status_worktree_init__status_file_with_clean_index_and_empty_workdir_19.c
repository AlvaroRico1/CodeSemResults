void test_status_worktree_init__status_file_with_clean_index_and_empty_workdir(void)
{
	git_repository *repo;
	unsigned int status = 0;
	git_index *index;

	cl_git_pass(p_mkdir("wd", 0777));

	cl_git_pass(git_repository_open(&repo, cl_fixture("testrepo.git")));
	cl_git_pass(git_repository_set_workdir(repo, "wd", false));

	cl_git_pass(git_index_open(&index, "my-index"));
	fill_index_wth_head_entries(repo, index);

	git_repository_set_index(repo, index);

	cl_git_pass(git_status_file(&status, repo, "branch_file.txt"));

	cl_assert_equal_i(GIT_STATUS_WT_DELETED, status);

	git_repository_free(repo);
	git_index_free(index);
	cl_git_pass(p_rmdir("wd"));
	cl_git_pass(p_unlink("my-index"));
}


// Source: worktree_init.c
// Lines 98-122
