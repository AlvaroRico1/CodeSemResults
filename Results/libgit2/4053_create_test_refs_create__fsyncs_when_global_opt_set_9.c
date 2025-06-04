void test_refs_create__fsyncs_when_global_opt_set(void)
{
	size_t create_count, compress_count;

	cl_git_pass(git_libgit2_opts(GIT_OPT_ENABLE_FSYNC_GITDIR, 1));
	count_fsyncs(&create_count, &compress_count);

	cl_assert_equal_i(expected_fsyncs_create, create_count);
	cl_assert_equal_i(expected_fsyncs_compress, compress_count);
}


// Source: create.c
// Lines 341-350
