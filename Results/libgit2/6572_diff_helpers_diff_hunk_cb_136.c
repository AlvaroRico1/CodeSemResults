int diff_hunk_cb(
	const git_diff_delta *delta,
	const git_diff_hunk *hunk,
	void *payload)
{
	diff_expects *e = payload;
	const char *scan = hunk->header, *scan_end = scan + hunk->header_len;

	GIT_UNUSED(delta);

	/* confirm no NUL bytes in header text */
	while (scan < scan_end)
		cl_assert('\0' != *scan++);

	e->hunks++;
	e->hunk_old_lines += hunk->old_lines;
	e->hunk_new_lines += hunk->new_lines;
	return 0;
}


// Source: diff_helpers.c
// Lines 106-124
