void test_diff_blob__can_correctly_detect_textual_blob_data_as_non_binary(void)
{
	/* tests/resources/attr/root_test4.txt */
	const char *content = git_blob_rawcontent(d);
	size_t len = (size_t)git_blob_rawsize(d);
	cl_assert_equal_i(false, git_blob_data_is_binary(content, len));
}


// Source: blob.c
// Lines 621-627
