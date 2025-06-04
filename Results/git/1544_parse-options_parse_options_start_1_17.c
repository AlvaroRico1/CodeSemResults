static void parse_options_start_1(struct parse_opt_ctx_t *ctx,
				  int argc, const char **argv, const char *prefix,
				  const struct option *options, int flags)
{
	ctx->argc = argc;
	ctx->argv = argv;
	if (!(flags & PARSE_OPT_ONE_SHOT)) {
		ctx->argc--;
		ctx->argv++;
	}
	ctx->total = ctx->argc;
	ctx->out   = argv;
	ctx->prefix = prefix;
	ctx->cpidx = ((flags & PARSE_OPT_KEEP_ARGV0) != 0);
	ctx->flags = flags;
	if ((flags & PARSE_OPT_KEEP_UNKNOWN) &&
	    (flags & PARSE_OPT_STOP_AT_NON_OPTION) &&
	    !(flags & PARSE_OPT_ONE_SHOT))
		BUG("STOP_AT_NON_OPTION and KEEP_UNKNOWN don't go together");
	if ((flags & PARSE_OPT_ONE_SHOT) &&
	    (flags & PARSE_OPT_KEEP_ARGV0))
		BUG("Can't keep argv0 if you don't have it");
	parse_options_check(options);
}


// Source: parse-options.c
// Lines 482-505
