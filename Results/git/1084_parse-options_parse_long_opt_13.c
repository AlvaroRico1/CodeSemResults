static enum parse_opt_result parse_long_opt(
	struct parse_opt_ctx_t *p, const char *arg,
	const struct option *options)
{
	const struct option *all_opts = options;
	const char *arg_end = strchrnul(arg, '=');
	const struct option *abbrev_option = NULL, *ambiguous_option = NULL;
	int abbrev_flags = 0, ambiguous_flags = 0;

	for (; options->type != OPTION_END; options++) {
		const char *rest, *long_name = options->long_name;
		int flags = 0, opt_flags = 0;

		if (!long_name)
			continue;

again:
		if (!skip_prefix(arg, long_name, &rest))
			rest = NULL;
		if (!rest) {
			/* abbreviated? */
			if (!(p->flags & PARSE_OPT_KEEP_UNKNOWN) &&
			    !strncmp(long_name, arg, arg_end - arg)) {
is_abbreviated:
				if (abbrev_option &&
				    !is_alias(p, abbrev_option, options)) {
					/*
					 * If this is abbreviated, it is
					 * ambiguous. So when there is no
					 * exact match later, we need to
					 * error out.
					 */
					ambiguous_option = abbrev_option;
					ambiguous_flags = abbrev_flags;
				}


// Source: parse-options.c
// Lines 294-328
