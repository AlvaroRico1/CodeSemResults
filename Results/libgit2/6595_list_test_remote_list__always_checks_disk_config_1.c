void test_remote_list__always_checks_disk_config(void)
{
	git_repository *repo;
	git_strarray remotes;
	git_remote *remote;

	cl_git_pass(git_repository_open(&repo, git_repository_path(_repo)));

	cl_git_pass(git_remote_list(&remotes, _repo));
	cl_assert_equal_sz(remotes.count, 1);
	git_strarray_dispose(&remotes);

	cl_git_pass(git_remote_create(&remote, _repo, "valid-name", TEST_URL));

	cl_git_pass(git_remote_list(&remotes, _repo));
	cl_assert_equal_sz(remotes.count, 2);
	git_strarray_dispose(&remotes);

	cl_git_pass(git_remote_list(&remotes, repo));
	cl_assert_equal_sz(remotes.count, 2);
	git_strarray_dispose(&remotes);

	git_repository_free(repo);
	git_remote_free(remote);
}


// Source: list.c
// Lines 18-42
