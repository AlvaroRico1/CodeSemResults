void test_diff_blob__can_correctly_detect_binary_blob_data_as_binary(void)
{
	/* alien.png */
	const char *content = git_blob_rawcontent(alien);
	size_t len = (size_t)git_blob_rawsize(alien);
	cl_assert_equal_i(true, git_blob_data_is_binary(content, len));
}


// Source: blob.c
// Lines 607-613
