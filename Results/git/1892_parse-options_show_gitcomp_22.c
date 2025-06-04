static int show_gitcomp(const struct option *opts, int show_all)
{
	const struct option *original_opts = opts;
	int nr_noopts = 0;

	for (; opts->type != OPTION_END; opts++) {
		const char *suffix = "";

		if (!opts->long_name)
			continue;
		if (!show_all &&
			(opts->flags & (PARSE_OPT_HIDDEN | PARSE_OPT_NOCOMPLETE | PARSE_OPT_FROM_ALIAS)))
			continue;

		switch (opts->type) {
		case OPTION_GROUP:
			continue;
		case OPTION_STRING:
		case OPTION_FILENAME:
		case OPTION_INTEGER:
		case OPTION_MAGNITUDE:
		case OPTION_CALLBACK:
			if (opts->flags & PARSE_OPT_NOARG)
				break;
			if (opts->flags & PARSE_OPT_OPTARG)
				break;
			if (opts->flags & PARSE_OPT_LASTARG_DEFAULT)
				break;
			suffix = "=";
			break;
		default:
			break;
		}


// Source: parse-options.c
// Lines 564-596
