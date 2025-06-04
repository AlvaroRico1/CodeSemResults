void test_diff_parse__no_extended_headers(void)
{
	const char *content = PATCH_NO_EXTENDED_HEADERS;
	git_diff *diff;

	cl_git_pass(git_diff_from_buffer(
		&diff, content, strlen(content)));
	git_diff_free(diff);
}


// Source: parse.c
// Lines 100-108
