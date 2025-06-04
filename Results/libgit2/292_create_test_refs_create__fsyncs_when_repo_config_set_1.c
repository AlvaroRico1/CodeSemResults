void test_refs_create__fsyncs_when_repo_config_set(void)
{
	size_t create_count, compress_count;

	cl_repo_set_bool(g_repo, "core.fsyncObjectFiles", true);

	count_fsyncs(&create_count, &compress_count);

	cl_assert_equal_i(expected_fsyncs_create, create_count);
	cl_assert_equal_i(expected_fsyncs_compress, compress_count);
}


// Source: create.c
// Lines 352-362
