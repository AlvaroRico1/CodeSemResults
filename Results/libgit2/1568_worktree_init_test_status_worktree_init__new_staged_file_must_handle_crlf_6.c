void test_status_worktree_init__new_staged_file_must_handle_crlf(void)
{
	git_repository *repo;
	git_index *index;
	unsigned int status;

	cl_set_cleanup(&cleanup_new_repo, "getting_started");
	cl_git_pass(git_repository_init(&repo, "getting_started", 0));

	/* Ensure that repo has core.autocrlf=true */
	cl_repo_set_bool(repo, "core.autocrlf", true);

	cl_git_mkfile("getting_started/testfile.txt", "content\r\n");	/* Content with CRLF */

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_index_add_bypath(index, "testfile.txt"));
	cl_git_pass(git_index_write(index));

	cl_git_pass(git_status_file(&status, repo, "testfile.txt"));
	cl_assert_equal_i(GIT_STATUS_INDEX_NEW, status);

	git_index_free(index);
	git_repository_free(repo);
}


// Source: worktree_init.c
// Lines 314-337
