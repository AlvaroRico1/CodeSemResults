static size_t num_modified_deltas(git_diff *diff)
{
	const git_diff_delta *delta;
	size_t i, cnt = 0;

	for (i = 0; i < git_diff_num_deltas(diff); i++) {
		delta = git_diff_get_delta(diff, i);

		if (delta->status != GIT_DELTA_UNMODIFIED)
			cnt++;
	}

	return cnt;
}


// Source: diff_helpers.c
// Lines 245-258
