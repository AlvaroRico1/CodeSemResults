static int notify_cb__racy_rmdir(
	const git_diff *diff_so_far,
	const git_diff_delta *delta_to_add,
	const char *matched_pathspec,
	void *payload)
{
	racy_payload *pay = (racy_payload *)payload;

	if (pay->first_time) {
		cl_must_pass(p_rmdir(pay->dir));
		pay->first_time = false;
	}

	return notify_cb__basic(diff_so_far, delta_to_add, matched_pathspec, pay->basic_payload);
}


// Source: racediffiter.c
// Lines 90-104
