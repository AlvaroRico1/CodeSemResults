void test_attr_flags__index_vs_workdir(void)
{
	git_repository *repo = cl_git_sandbox_init("attr_index");
	const char *value;

	cl_assert(!git_repository_is_bare(repo));

	/* wd then index */
	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_FILE_THEN_INDEX,
		"README.md", "bar"));
	cl_assert(GIT_ATTR_IS_FALSE(value));

	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_FILE_THEN_INDEX,
		"README.md", "blargh"));
	cl_assert_equal_s(value, "goop");

	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_FILE_THEN_INDEX,
		"README.txt", "foo"));
	cl_assert(GIT_ATTR_IS_FALSE(value));

	/* index then wd */
	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_INDEX_THEN_FILE,
		"README.md", "bar"));
	cl_assert(GIT_ATTR_IS_TRUE(value));

	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_INDEX_THEN_FILE,
		"README.md", "blargh"));
	cl_assert_equal_s(value, "garble");

	cl_git_pass(git_attr_get(
		&value, repo, GIT_ATTR_CHECK_NO_SYSTEM | GIT_ATTR_CHECK_INDEX_THEN_FILE,
		"README.txt", "foo"));
	cl_assert(GIT_ATTR_IS_TRUE(value));
}


// Source: flags.c
// Lines 21-59
