static int test_checkout_notify_cb(
	git_checkout_notify_t why,
	const char *path,
	const git_diff_file *baseline,
	const git_diff_file *target,
	const git_diff_file *workdir,
	void *payload)
{
	struct notify_data *expectations = (struct notify_data *)payload;

	GIT_UNUSED(workdir);

	cl_assert_equal_i(GIT_CHECKOUT_NOTIFY_CONFLICT, why);
	cl_assert_equal_s(expectations->file, path);
	cl_assert_equal_i(0, git_oid_streq(&baseline->id, expectations->sha));
	cl_assert_equal_i(0, git_oid_streq(&target->id, expectations->sha));

	return 0;
}


// Source: index.c
// Lines 452-470
