static int normalize_find_opts(
	git_diff *diff,
	git_diff_find_options *opts,
	const git_diff_find_options *given)
{
	git_config *cfg = NULL;
	git_hashsig_option_t hashsig_opts;

	GIT_ERROR_CHECK_VERSION(given, GIT_DIFF_FIND_OPTIONS_VERSION, "git_diff_find_options");

	if (diff->repo != NULL &&
		git_repository_config__weakptr(&cfg, diff->repo) < 0)
		return -1;

	if (given)
		memcpy(opts, given, sizeof(*opts));

	if (!given ||
		 (given->flags & GIT_DIFF_FIND_ALL) == GIT_DIFF_FIND_BY_CONFIG)
	{
		if (cfg) {
			char *rule =
				git_config__get_string_force(cfg, "diff.renames", "true");
			int boolval;

			if (!git__parse_bool(&boolval, rule) && !boolval)
				/* don't set FIND_RENAMES if bool value is false */;
			else if (!strcasecmp(rule, "copies") || !strcasecmp(rule, "copy"))
				opts->flags |= GIT_DIFF_FIND_RENAMES | GIT_DIFF_FIND_COPIES;
			else
				opts->flags |= GIT_DIFF_FIND_RENAMES;

			git__free(rule);
		} else {
			/* set default flag */
			opts->flags |= GIT_DIFF_FIND_RENAMES;
		}
	}

	/* some flags imply others */

	if (opts->flags & GIT_DIFF_FIND_EXACT_MATCH_ONLY) {
		/* if we are only looking for exact matches, then don't turn
		 * MODIFIED items into ADD/DELETE pairs because it's too picky
		 */
		opts->flags &= ~(GIT_DIFF_FIND_REWRITES | GIT_DIFF_BREAK_REWRITES);

		/* similarly, don't look for self-rewrites to split */
		opts->flags &= ~GIT_DIFF_FIND_RENAMES_FROM_REWRITES;
	}

	if (opts->flags & GIT_DIFF_FIND_RENAMES_FROM_REWRITES)
		opts->flags |= GIT_DIFF_FIND_RENAMES;

	if (opts->flags & GIT_DIFF_FIND_COPIES_FROM_UNMODIFIED)
		opts->flags |= GIT_DIFF_FIND_COPIES;

	if (opts->flags & GIT_DIFF_BREAK_REWRITES)
		opts->flags |= GIT_DIFF_FIND_REWRITES;

#define USE_DEFAULT(X) ((X) == 0 || (X) > 100)

	if (USE_DEFAULT(opts->rename_threshold))
		opts->rename_threshold = DEFAULT_THRESHOLD;

	if (USE_DEFAULT(opts->rename_from_rewrite_threshold))
		opts->rename_from_rewrite_threshold = DEFAULT_THRESHOLD;

	if (USE_DEFAULT(opts->copy_threshold))
		opts->copy_threshold = DEFAULT_THRESHOLD;

	if (USE_DEFAULT(opts->break_rewrite_threshold))
		opts->break_rewrite_threshold = DEFAULT_BREAK_REWRITE_THRESHOLD;

#undef USE_DEFAULT

	if (!opts->rename_limit) {
		if (cfg) {
			opts->rename_limit = git_config__get_int_force(
				cfg, "diff.renamelimit", DEFAULT_RENAME_LIMIT);
		}

		if (opts->rename_limit <= 0)
			opts->rename_limit = DEFAULT_RENAME_LIMIT;
	}

	/* assign the internal metric with whitespace flag as payload */
	if (!opts->metric) {
		opts->metric = git__malloc(sizeof(git_diff_similarity_metric));
		GIT_ERROR_CHECK_ALLOC(opts->metric);

		opts->metric->file_signature = git_diff_find_similar__hashsig_for_file;
		opts->metric->buffer_signature = git_diff_find_similar__hashsig_for_buf;
		opts->metric->free_signature = git_diff_find_similar__hashsig_free;
		opts->metric->similarity = git_diff_find_similar__calc_similarity;

		if (opts->flags & GIT_DIFF_FIND_IGNORE_WHITESPACE)
			hashsig_opts = GIT_HASHSIG_IGNORE_WHITESPACE;
		else if (opts->flags & GIT_DIFF_FIND_DONT_IGNORE_WHITESPACE)
			hashsig_opts = GIT_HASHSIG_NORMAL;
		else
			hashsig_opts = GIT_HASHSIG_SMART_WHITESPACE;
		hashsig_opts |= GIT_HASHSIG_ALLOW_SMALL_FILES;
		opts->metric->payload = (void *)hashsig_opts;
	}

	return 0;
}


// Source: diff_tform.c
// Lines 246-353
