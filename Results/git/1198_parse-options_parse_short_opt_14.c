static enum parse_opt_result parse_short_opt(struct parse_opt_ctx_t *p,
					     const struct option *options)
{
	const struct option *all_opts = options;
	const struct option *numopt = NULL;

	for (; options->type != OPTION_END; options++) {
		if (options->short_name == *p->opt) {
			p->opt = p->opt[1] ? p->opt + 1 : NULL;
			return get_value(p, options, all_opts, OPT_SHORT);
		}

		/*
		 * Handle the numerical option later, explicit one-digit
		 * options take precedence over it.
		 */
		if (options->type == OPTION_NUMBER)
			numopt = options;
	}
	if (numopt && isdigit(*p->opt)) {
		size_t len = 1;
		char *arg;
		int rc;

		while (isdigit(p->opt[len]))
			len++;
		arg = xmemdupz(p->opt, len);
		p->opt = p->opt[len] ? p->opt + len : NULL;
		if (numopt->callback)
			rc = (*numopt->callback)(numopt, arg, 0) ? (-1) : 0;
		else
			rc = (*numopt->ll_callback)(p, numopt, arg, 0);
		free(arg);
		return rc;
	}


// Source: parse-options.c
// Lines 227-261
