void test_submodule_inject_option__path(void)
{
	int foundit;
	git_submodule *sm;
	git_str buf = GIT_STR_INIT;

	cl_git_pass(git_str_joinpath(&buf, git_repository_workdir(g_repo), ".gitmodules"));
	cl_git_rewritefile(buf.ptr,
			   "[submodule \"naughty\"]\n"
			   "    path = --something\n"
			   "    url = blah.git\n");
	git_str_dispose(&buf);

	/* We do want to find it, but with the appropriate field empty */
	foundit = 0;
	cl_git_pass(git_submodule_foreach(g_repo, find_naughty, &foundit));
	cl_assert_equal_i(1, foundit);

	cl_git_pass(git_submodule_lookup(&sm, g_repo, "naughty"));
	cl_assert_equal_s("naughty", git_submodule_path(sm));
	cl_assert_equal_s("blah.git", git_submodule_url(sm));

	git_submodule_free(sm);
}


// Source: inject_option.c
// Lines 57-80
