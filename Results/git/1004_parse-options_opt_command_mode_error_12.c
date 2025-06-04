static enum parse_opt_result opt_command_mode_error(
	const struct option *opt,
	const struct option *all_opts,
	int flags)
{
	const struct option *that;
	struct strbuf that_name = STRBUF_INIT;

	/*
	 * Find the other option that was used to set the variable
	 * already, and report that this is not compatible with it.
	 */
	for (that = all_opts; that->type != OPTION_END; that++) {
		if (that == opt ||
		    !(that->flags & PARSE_OPT_CMDMODE) ||
		    that->value != opt->value ||
		    that->defval != *(int *)opt->value)
			continue;

		if (that->long_name)
			strbuf_addf(&that_name, "--%s", that->long_name);
		else
			strbuf_addf(&that_name, "-%c", that->short_name);
		error(_("%s is incompatible with %s"),
		      optname(opt, flags), that_name.buf);
		strbuf_release(&that_name);
		return PARSE_OPT_ERROR;
	}


// Source: parse-options.c
// Lines 50-77
