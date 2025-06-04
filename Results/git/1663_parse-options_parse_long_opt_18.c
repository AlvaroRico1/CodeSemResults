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
				if (!(flags & OPT_UNSET) && *arg_end)
					p->opt = arg_end + 1;
				abbrev_option = options;
				abbrev_flags = flags ^ opt_flags;
				continue;
			}
			/* negation allowed? */
			if (options->flags & PARSE_OPT_NONEG)
				continue;
			/* negated and abbreviated very much? */
			if (starts_with("no-", arg)) {
				flags |= OPT_UNSET;
				goto is_abbreviated;
			}
			/* negated? */
			if (!starts_with(arg, "no-")) {
				if (skip_prefix(long_name, "no-", &long_name)) {
					opt_flags |= OPT_UNSET;
					goto again;
				}
				continue;
			}
			flags |= OPT_UNSET;
			if (!skip_prefix(arg + 3, long_name, &rest)) {
				/* abbreviated and negated? */
				if (starts_with(long_name, arg + 3))
					goto is_abbreviated;
				else
					continue;
			}
		}
		if (*rest) {
			if (*rest != '=')
				continue;
			p->opt = rest + 1;
		}
		return get_value(p, options, all_opts, flags ^ opt_flags);
	}

	if (disallow_abbreviated_options && (ambiguous_option || abbrev_option))
		die("disallowed abbreviated or ambiguous option '%.*s'",
		    (int)(arg_end - arg), arg);

	if (ambiguous_option) {
		error(_("ambiguous option: %s "
			"(could be --%s%s or --%s%s)"),
			arg,
			(ambiguous_flags & OPT_UNSET) ?  "no-" : "",
			ambiguous_option->long_name,
			(abbrev_flags & OPT_UNSET) ?  "no-" : "",
			abbrev_option->long_name);
		return PARSE_OPT_HELP;
	}
	if (abbrev_option)
		return get_value(p, abbrev_option, all_opts, abbrev_flags);
	return PARSE_OPT_UNKNOWN;
}


// Source: parse-options.c
// Lines 294-385
