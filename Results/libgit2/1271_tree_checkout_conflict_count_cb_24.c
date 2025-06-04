static int checkout_conflict_count_cb(
	git_checkout_notify_t why,
	const char *path,
	const git_diff_file *b,
	const git_diff_file *t,
	const git_diff_file *w,
	void *payload)
{
	size_t *n = payload;

	GIT_UNUSED(why);
	GIT_UNUSED(path);
	GIT_UNUSED(b);
	GIT_UNUSED(t);
	GIT_UNUSED(w);

	(*n)++;

	return 0;
}


// Source: tree.c
// Lines 1470-1489
