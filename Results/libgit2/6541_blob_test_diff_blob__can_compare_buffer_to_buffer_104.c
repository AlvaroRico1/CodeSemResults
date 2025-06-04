void test_diff_blob__can_compare_buffer_to_buffer(void)
{
	const char *a = "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\n";
	const char *b = "a\nB\nc\nd\nE\nF\nh\nj\nk\n";

	opts.interhunk_lines = 0;
	opts.context_lines = 0;

	memset(&expected, 0, sizeof(expected));

	cl_git_pass(git_diff_buffers(
		a, strlen(a), NULL, b, strlen(b), NULL, &opts,
		diff_file_cb, diff_binary_cb, diff_hunk_cb, diff_line_cb, &expected));
	assert_one_modified(4, 9, 0, 4, 5, &expected);

	opts.flags ^= GIT_DIFF_REVERSE;

	memset(&expected, 0, sizeof(expected));

	cl_git_pass(git_diff_buffers(
		a, strlen(a), NULL, b, strlen(b), NULL, &opts,
		diff_file_cb, diff_binary_cb, diff_hunk_cb, diff_line_cb, &expected));
	assert_one_modified(4, 9, 0, 5, 4, &expected);
}


// Source: blob.c
// Lines 1040-1063
