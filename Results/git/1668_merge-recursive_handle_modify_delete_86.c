static int handle_modify_delete(struct merge_options *opt,
				const char *path,
				const struct diff_filespec *o,
				const struct diff_filespec *a,
				const struct diff_filespec *b)
{
	const char *modify_branch, *delete_branch;
	const struct diff_filespec *changed;

	if (is_valid(a)) {
		modify_branch = opt->branch1;
		delete_branch = opt->branch2;
		changed = a;
	} else {
		modify_branch = opt->branch2;
		delete_branch = opt->branch1;
		changed = b;
	}

	return handle_change_delete(opt,
				    path, NULL,
				    o, changed,
				    modify_branch, delete_branch,
				    _("modify"), _("modified"));
}


// Source: merge-recursive.c
// Lines 3067-3091
