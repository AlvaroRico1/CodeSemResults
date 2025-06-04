void test_refs_create__does_not_fsync_by_default(void)
{
	size_t create_count, compress_count;
	count_fsyncs(&create_count, &compress_count);

	cl_assert_equal_i(0, create_count);
	cl_assert_equal_i(0, compress_count);
}


// Source: create.c
// Lines 332-339
