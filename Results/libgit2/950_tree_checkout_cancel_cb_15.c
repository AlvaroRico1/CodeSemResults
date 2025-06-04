static int checkout_cancel_cb(
	git_checkout_notify_t why,
	const char *path,
	const git_diff_file *b,
	const git_diff_file *t,
	const git_diff_file *w,
	void *payload)
{
	struct checkout_cancel_at *ca = payload;

	GIT_UNUSED(why); GIT_UNUSED(b); GIT_UNUSED(t); GIT_UNUSED(w);

	ca->count++;

	if (!strcmp(path, ca->filename))
		return ca->error;

	return 0;
}


// Source: tree.c
// Lines 648-666
