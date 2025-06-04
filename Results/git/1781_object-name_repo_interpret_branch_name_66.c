int repo_interpret_branch_name(struct repository *r,
			       const char *name, int namelen,
			       struct strbuf *buf,
			       const struct interpret_branch_name_options *options)
{
	char *at;
	const char *start;
	int len;

	if (!namelen)
		namelen = strlen(name);

	if (!options->allowed || (options->allowed & INTERPRET_BRANCH_LOCAL)) {
		len = interpret_nth_prior_checkout(r, name, namelen, buf);
		if (!len) {
			return len; /* syntax Ok, not enough switches */
		} else if (len > 0) {
			if (len == namelen)
				return len; /* consumed all */
			else
				return reinterpret(r, name, namelen, len, buf,
						   options->allowed);
		}
	}

	for (start = name;
	     (at = memchr(start, '@', namelen - (start - name)));
	     start = at + 1) {

		if (!options->allowed || (options->allowed & INTERPRET_BRANCH_HEAD)) {
			len = interpret_empty_at(name, namelen, at - name, buf);
			if (len > 0)
				return reinterpret(r, name, namelen, len, buf,
						   options->allowed);
		}

		len = interpret_branch_mark(r, name, namelen, at - name, buf,
					    upstream_mark, branch_get_upstream,
					    options);
		if (len > 0)
			return len;

		len = interpret_branch_mark(r, name, namelen, at - name, buf,
					    push_mark, branch_get_push,
					    options);
		if (len > 0)
			return len;
	}

	return -1;
}


// Source: object-name.c
// Lines 1509-1559
