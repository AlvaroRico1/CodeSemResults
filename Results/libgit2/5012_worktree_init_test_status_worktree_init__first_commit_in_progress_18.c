void test_status_worktree_init__first_commit_in_progress(void)
{
	git_repository *repo;
	git_index *index;
	status_entry_single result;

	cl_set_cleanup(&cleanup_new_repo, "getting_started");

	cl_git_pass(git_repository_init(&repo, "getting_started", 0));
	cl_git_mkfile("getting_started/testfile.txt", "content\n");

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(1, result.count);
	cl_assert(result.status == GIT_STATUS_WT_NEW);

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_index_add_bypath(index, "testfile.txt"));
	cl_git_pass(git_index_write(index));

	memset(&result, 0, sizeof(result));
	cl_git_pass(git_status_foreach(repo, cb_status__single, &result));
	cl_assert_equal_i(1, result.count);
	cl_assert(result.status == GIT_STATUS_INDEX_NEW);

	git_index_free(index);
	git_repository_free(repo);
}


// Source: worktree_init.c
// Lines 26-53
