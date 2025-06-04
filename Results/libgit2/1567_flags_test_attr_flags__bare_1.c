void test_attr_flags__bare(void)
{
	git_repository *repo = cl_git_sandbox_init("testrepo.git");
	const char *value;

	cl_assert(git_repository_is_bare(repo));

	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM, "README.md", "diff"));
	cl_assert(GIT_ATTR_IS_UNSPECIFIED(value));
}


// Source: flags.c
// Lines 9-19
