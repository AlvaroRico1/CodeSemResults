int parse_options_step(struct parse_opt_ctx_t *ctx,
		       const struct option *options,
		       const char * const usagestr[])
{
	int internal_help = !(ctx->flags & PARSE_OPT_NO_INTERNAL_HELP);

	/* we must reset ->opt, unknown short option leave it dangling */
	ctx->opt = NULL;

	for (; ctx->argc; ctx->argc--, ctx->argv++) {
		const char *arg = ctx->argv[0];

		if (ctx->flags & PARSE_OPT_ONE_SHOT &&
		    ctx->argc != ctx->total)
			break;

		if (*arg != '-' || !arg[1]) {
			if (parse_nodash_opt(ctx, arg, options) == 0)
				continue;
			if (ctx->flags & PARSE_OPT_STOP_AT_NON_OPTION)
				return PARSE_OPT_NON_OPTION;
			ctx->out[ctx->cpidx++] = ctx->argv[0];
			continue;
		}

		/* lone -h asks for help */
		if (internal_help && ctx->total == 1 && !strcmp(arg + 1, "h"))
			goto show_usage;

		/*
		 * lone --git-completion-helper and --git-completion-helper-all
		 * are asked by git-completion.bash
		 */
		if (ctx->total == 1 && !strcmp(arg, "--git-completion-helper"))
			return show_gitcomp(options, 0);
		if (ctx->total == 1 && !strcmp(arg, "--git-completion-helper-all"))
			return show_gitcomp(options, 1);

		if (arg[1] != '-') {
			ctx->opt = arg + 1;
			switch (parse_short_opt(ctx, options)) {
			case PARSE_OPT_ERROR:
				return PARSE_OPT_ERROR;
			case PARSE_OPT_UNKNOWN:
				if (ctx->opt)
					check_typos(arg + 1, options);
				if (internal_help && *ctx->opt == 'h')
					goto show_usage;
				goto unknown;
			case PARSE_OPT_NON_OPTION:
			case PARSE_OPT_HELP:
			case PARSE_OPT_COMPLETE:
				BUG("parse_short_opt() cannot return these");
			case PARSE_OPT_DONE:
				break;
			}
			if (ctx->opt)
				check_typos(arg + 1, options);
			while (ctx->opt) {
				switch (parse_short_opt(ctx, options)) {
				case PARSE_OPT_ERROR:
					return PARSE_OPT_ERROR;
				case PARSE_OPT_UNKNOWN:
					if (internal_help && *ctx->opt == 'h')
						goto show_usage;

					/* fake a short option thing to hide the fact that we may have
					 * started to parse aggregated stuff
					 *
					 * This is leaky, too bad.
					 */
					ctx->argv[0] = xstrdup(ctx->opt - 1);
					*(char *)ctx->argv[0] = '-';
					goto unknown;
				case PARSE_OPT_NON_OPTION:
				case PARSE_OPT_COMPLETE:
				case PARSE_OPT_HELP:
					BUG("parse_short_opt() cannot return these");
				case PARSE_OPT_DONE:
					break;
				}
			}
			continue;
		}

		if (!arg[2] /* "--" */ ||
		    !strcmp(arg + 2, "end-of-options")) {
			if (!(ctx->flags & PARSE_OPT_KEEP_DASHDASH)) {
				ctx->argc--;
				ctx->argv++;
			}
			break;
		}

		if (internal_help && !strcmp(arg + 2, "help-all"))
			return usage_with_options_internal(ctx, usagestr, options, 1, 0);
		if (internal_help && !strcmp(arg + 2, "help"))
			goto show_usage;
		switch (parse_long_opt(ctx, arg + 2, options)) {
		case PARSE_OPT_ERROR:
			return PARSE_OPT_ERROR;
		case PARSE_OPT_UNKNOWN:
			goto unknown;
		case PARSE_OPT_HELP:
			goto show_usage;
		case PARSE_OPT_NON_OPTION:
		case PARSE_OPT_COMPLETE:
			BUG("parse_long_opt() cannot return these");
		case PARSE_OPT_DONE:
			break;
		}
		continue;
unknown:
		if (ctx->flags & PARSE_OPT_ONE_SHOT)
			break;
		if (!(ctx->flags & PARSE_OPT_KEEP_UNKNOWN))
			return PARSE_OPT_UNKNOWN;
		ctx->out[ctx->cpidx++] = ctx->argv[0];
		ctx->opt = NULL;
	}


// Source: parse-options.c
// Lines 704-823
