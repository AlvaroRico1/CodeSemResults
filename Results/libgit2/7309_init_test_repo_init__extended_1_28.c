void test_repo_init__extended_1(void)
{
	git_reference *ref;
	git_remote *remote;
	struct stat st;
	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;

	opts.flags = GIT_REPOSITORY_INIT_MKPATH |
		GIT_REPOSITORY_INIT_NO_DOTGIT_DIR;
	opts.mode = GIT_REPOSITORY_INIT_SHARED_GROUP;
	opts.workdir_path = "../c_wd";
	opts.description = "Awesomest test repository evah";
	opts.initial_head = "development";
	opts.origin_url = "https://github.com/libgit2/libgit2.git";

	cl_git_pass(git_repository_init_ext(&g_repo, "root/b/c.git", &opts));

	cl_assert(!git__suffixcmp(git_repository_workdir(g_repo), "/c_wd/"));
	cl_assert(!git__suffixcmp(git_repository_path(g_repo), "/c.git/"));
	cl_assert(git_fs_path_isfile("root/b/c_wd/.git"));
	cl_assert(!git_repository_is_bare(g_repo));
	/* repo will not be counted as empty because we set head to "development" */
	cl_assert(!git_repository_is_empty(g_repo));

	cl_git_pass(git_fs_path_lstat(git_repository_path(g_repo), &st));
	cl_assert(S_ISDIR(st.st_mode));
	if (cl_is_chmod_supported())
		cl_assert((S_ISGID & st.st_mode) == S_ISGID);
	else
		cl_assert((S_ISGID & st.st_mode) == 0);

	cl_git_pass(git_reference_lookup(&ref, g_repo, "HEAD"));
	cl_assert(git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC);
	cl_assert_equal_s("refs/heads/development", git_reference_symbolic_target(ref));
	git_reference_free(ref);

	cl_git_pass(git_remote_lookup(&remote, g_repo, "origin"));
	cl_assert_equal_s("origin", git_remote_name(remote));
	cl_assert_equal_s(opts.origin_url, git_remote_url(remote));
	git_remote_free(remote);

	git_repository_free(g_repo);
	cl_fixture_cleanup("root");
}


// Source: init.c
// Lines 402-445
