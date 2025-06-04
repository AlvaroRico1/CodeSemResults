static int interpret_nth_prior_checkout(struct repository *r,
					const char *name, int namelen,
					struct strbuf *buf)
{
	long nth;
	int retval;
	struct grab_nth_branch_switch_cbdata cb;
	const char *brace;
	char *num_end;

	if (namelen < 4)
		return -1;
	if (name[0] != '@' || name[1] != '{' || name[2] != '-')
		return -1;
	brace = memchr(name, '}', namelen);
	if (!brace)
		return -1;
	nth = strtol(name + 3, &num_end, 10);
	if (num_end != brace)
		return -1;
	if (nth <= 0)
		return -1;
	cb.remaining = nth;
	cb.sb = buf;

	retval = refs_for_each_reflog_ent_reverse(get_main_ref_store(r),
			"HEAD", grab_nth_branch_switch, &cb);
	if (0 < retval) {
		retval = brace - name + 1;
	} else
		retval = 0;

	return retval;
}


// Source: object-name.c
// Lines 1315-1348
