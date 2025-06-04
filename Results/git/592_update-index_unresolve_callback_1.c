static enum parse_opt_result unresolve_callback(
	struct parse_opt_ctx_t *ctx, const struct option *opt,
	const char *arg, int unset)
{
	int *has_errors = opt->value;
	const char *prefix = startup_info->prefix;

	BUG_ON_OPT_NEG(unset);
	BUG_ON_OPT_ARG(arg);

	/* consume remaining arguments. */
	*has_errors = do_unresolve(ctx->argc, ctx->argv,
				prefix, prefix ? strlen(prefix) : 0);
	if (*has_errors)
		active_cache_changed = 0;

	ctx->argv += ctx->argc - 1;
	ctx->argc = 1;
	return 0;
}


// Source: update-index.c
// Lines 912-931
