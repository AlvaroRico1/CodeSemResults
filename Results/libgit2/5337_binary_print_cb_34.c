static int print_cb(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	const git_diff_line *line,
	void *payload)
{
	git_str *buf = (git_str *)payload;

	GIT_UNUSED(delta);

	if (hunk)
		git_str_put(buf, hunk->header, hunk->header_len);

	if (line)
		git_str_put(buf, line->content, line->content_len);

	return git_str_oom(buf) ? -1 : 0;
}


// Source: binary.c
// Lines 355-372
