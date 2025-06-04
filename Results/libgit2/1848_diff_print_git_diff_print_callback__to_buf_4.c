int git_diff_print_callback__to_buf(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	const git_diff_line *line,
	void *payload)
{
	git_str *output = payload;
	GIT_UNUSED(delta); GIT_UNUSED(hunk);

	if (!output) {
		git_error_set(GIT_ERROR_INVALID, "buffer pointer must be provided");
		return -1;
	}

	if (line->origin == GIT_DIFF_LINE_ADDITION ||
	    line->origin == GIT_DIFF_LINE_DELETION ||
	    line->origin == GIT_DIFF_LINE_CONTEXT)
		git_str_putc(output, line->origin);

	return git_str_put(output, line->content, line->content_len);
}


// Source: diff_print.c
// Lines 712-732
