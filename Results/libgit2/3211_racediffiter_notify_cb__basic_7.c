static int notify_cb__basic(
	const git_diff *diff_so_far,
	const git_diff_delta *delta_to_add,
	const char *matched_pathspec,
	void *payload)
{
	basic_payload *exp = (basic_payload *)payload;
	basic_payload *e;

	GIT_UNUSED(diff_so_far);
	GIT_UNUSED(matched_pathspec);

	for (e = exp; e->path; e++) {
		if (strcmp(e->path, delta_to_add->new_file.path) == 0) {
			cl_assert_equal_i(e->t, delta_to_add->status);
			return 0;
		}
	}
	cl_assert(0);
	return GIT_ENOTFOUND;
}


// Source: racediffiter.c
// Lines 37-57
