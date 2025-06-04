void test_repo_reservedname__includes_shortname_when_requested(void)
{
	git_repository *repo;
	git_str *reserved;
	size_t reserved_len;

	repo = cl_git_sandbox_init("nasty");
	cl_assert(git_repository__reserved_names(&reserved, &reserved_len, repo, true));

	cl_assert_equal_i(2, reserved_len);
	cl_assert_equal_s(".git", reserved[0].ptr);
	cl_assert_equal_s("GIT~1", reserved[1].ptr);
}


// Source: reservedname.c
// Lines 29-41
