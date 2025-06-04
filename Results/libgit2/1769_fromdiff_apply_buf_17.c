static int apply_buf(
	const char *old,
	const char *oldname,
	const char *new,
	const char *newname,
	const char *patch_expected,
	const git_diff_options *diff_opts)
{
	git_str o = GIT_STR_INIT, n = GIT_STR_INIT,
		*optr = NULL, *nptr = NULL;

	if (old) {
		o.ptr = (char *)old;
		o.size = strlen(old);
		optr = &o;
	}

	if (new) {
		n.ptr = (char *)new;
		n.size = strlen(new);
		nptr = &n;
	}

	return apply_gitbuf(optr, oldname, nptr, newname, patch_expected, diff_opts);
}


// Source: fromdiff.c
// Lines 72-96
