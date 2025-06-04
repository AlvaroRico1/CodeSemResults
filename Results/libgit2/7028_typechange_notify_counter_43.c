static int notify_counter(
	git_checkout_notify_t why,
	const char *path,
	const git_diff_file *baseline,
	const git_diff_file *target,
	const git_diff_file *workdir,
	void *payload)
{
	notify_counts *cts = payload;

	GIT_UNUSED(path);
	GIT_UNUSED(baseline);
	GIT_UNUSED(target);
	GIT_UNUSED(workdir);

	switch (why) {
	case GIT_CHECKOUT_NOTIFY_CONFLICT:  cts->conflicts++; break;
	case GIT_CHECKOUT_NOTIFY_DIRTY:     cts->dirty++;     break;
	case GIT_CHECKOUT_NOTIFY_UPDATED:   cts->updates++;   break;
	case GIT_CHECKOUT_NOTIFY_UNTRACKED: cts->untracked++; break;
	case GIT_CHECKOUT_NOTIFY_IGNORED:   cts->ignored++;   break;
	default: break;
	}

	return 0;
}


// Source: typechange.c
// Lines 191-216
