int diff_file_cb(
	const git_diff_delta *delta,
	float progress,
	void *payload)
{
	diff_expects *e = payload;

	if (e->debug)
		fprintf_delta(stderr, delta, progress);

	if (e->names)
		cl_assert_equal_s(e->names[e->files], delta->old_file.path);
	if (e->statuses)
		cl_assert_equal_i(e->statuses[e->files], (int)delta->status);

	e->files++;

	if ((delta->flags & GIT_DIFF_FLAG_BINARY) != 0)
		e->files_binary++;

	cl_assert(delta->status <= GIT_DELTA_CONFLICTED);

	e->file_status[delta->status] += 1;

	return 0;
}


// Source: diff_helpers.c
// Lines 51-76
