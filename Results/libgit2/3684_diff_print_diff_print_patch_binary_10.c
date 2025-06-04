static int diff_print_patch_binary(
	const git_diff_delta *delta,
	const git_diff_binary *binary,
	void *data)
{
	diff_print_info *pi = data;
	const char *old_pfx =
		pi->old_prefix ? pi->old_prefix : DIFF_OLD_PREFIX_DEFAULT;
	const char *new_pfx =
		pi->new_prefix ? pi->new_prefix : DIFF_NEW_PREFIX_DEFAULT;
	int error;

	git_str_clear(pi->buf);

	if ((error = diff_print_patch_file_binary(
		pi, (git_diff_delta *)delta, old_pfx, new_pfx, binary)) < 0)
		return error;

	pi->line.origin = GIT_DIFF_LINE_BINARY;
	pi->line.content = git_str_cstr(pi->buf);
	pi->line.content_len = git_str_len(pi->buf);

	return pi->print_cb(delta, NULL, &pi->line, pi->payload);
}


// Source: diff_print.c
// Lines 599-622
