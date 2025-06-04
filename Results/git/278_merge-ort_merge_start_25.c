static void merge_start(struct merge_options *opt, struct merge_result *result)
{
	struct rename_info *renames;
	int i;
	struct mem_pool *pool = NULL;

	/* Sanity checks on opt */
	trace2_region_enter("merge", "sanity checks", opt->repo);
	assert(opt->repo);

	assert(opt->branch1 && opt->branch2);

	assert(opt->detect_directory_renames >= MERGE_DIRECTORY_RENAMES_NONE &&
	       opt->detect_directory_renames <= MERGE_DIRECTORY_RENAMES_TRUE);
	assert(opt->rename_limit >= -1);
	assert(opt->rename_score >= 0 && opt->rename_score <= MAX_SCORE);
	assert(opt->show_rename_progress >= 0 && opt->show_rename_progress <= 1);

	assert(opt->xdl_opts >= 0);
	assert(opt->recursive_variant >= MERGE_VARIANT_NORMAL &&
	       opt->recursive_variant <= MERGE_VARIANT_THEIRS);

	/*
	 * detect_renames, verbosity, buffer_output, and obuf are ignored
	 * fields that were used by "recursive" rather than "ort" -- but
	 * sanity check them anyway.
	 */
	assert(opt->detect_renames >= -1 &&
	       opt->detect_renames <= DIFF_DETECT_COPY);
	assert(opt->verbosity >= 0 && opt->verbosity <= 5);
	assert(opt->buffer_output <= 2);
	assert(opt->obuf.len == 0);

	assert(opt->priv == NULL);
	if (result->_properly_initialized != 0 &&
	    result->_properly_initialized != RESULT_INITIALIZED)
		BUG("struct merge_result passed to merge_incore_*recursive() must be zeroed or filled with values from a previous run");
	assert(!!result->priv == !!result->_properly_initialized);
	if (result->priv) {
		opt->priv = result->priv;
		result->priv = NULL;
		/*
		 * opt->priv non-NULL means we had results from a previous
		 * run; do a few sanity checks that user didn't mess with
		 * it in an obvious fashion.
		 */
		assert(opt->priv->call_depth == 0);
		assert(!opt->priv->toplevel_dir ||
		       0 == strlen(opt->priv->toplevel_dir));
	}
	trace2_region_leave("merge", "sanity checks", opt->repo);

	/* Default to histogram diff.  Actually, just hardcode it...for now. */
	opt->xdl_opts = DIFF_WITH_ALG(opt, HISTOGRAM_DIFF);

	/* Handle attr direction stuff for renormalization */
	if (opt->renormalize)
		git_attr_set_direction(GIT_ATTR_CHECKOUT);

	/* Initialization of opt->priv, our internal merge data */
	trace2_region_enter("merge", "allocate/init", opt->repo);
	if (opt->priv) {
		clear_or_reinit_internal_opts(opt->priv, 1);
		trace2_region_leave("merge", "allocate/init", opt->repo);
		return;
	}
	opt->priv = xcalloc(1, sizeof(*opt->priv));

	/* Initialization of various renames fields */
	renames = &opt->priv->renames;
	mem_pool_init(&opt->priv->pool, 0);
	pool = &opt->priv->pool;
	for (i = MERGE_SIDE1; i <= MERGE_SIDE2; i++) {
		strintmap_init_with_options(&renames->dirs_removed[i],
					    NOT_RELEVANT, pool, 0);
		strmap_init_with_options(&renames->dir_rename_count[i],
					 NULL, 1);
		strmap_init_with_options(&renames->dir_renames[i],
					 NULL, 0);
		/*
		 * relevant_sources uses -1 for the default, because we need
		 * to be able to distinguish not-in-strintmap from valid
		 * relevant_source values from enum file_rename_relevance.
		 * In particular, possibly_cache_new_pair() expects a negative
		 * value for not-found entries.
		 */
		strintmap_init_with_options(&renames->relevant_sources[i],
					    -1 /* explicitly invalid */,
					    pool, 0);
		strmap_init_with_options(&renames->cached_pairs[i],
					 NULL, 1);
		strset_init_with_options(&renames->cached_irrelevant[i],
					 NULL, 1);
		strset_init_with_options(&renames->cached_target_names[i],
					 NULL, 0);
	}
	for (i = MERGE_SIDE1; i <= MERGE_SIDE2; i++) {
		strintmap_init_with_options(&renames->deferred[i].possible_trivial_merges,
					    0, pool, 0);
		strset_init_with_options(&renames->deferred[i].target_dirs,
					 pool, 1);
		renames->deferred[i].trivial_merges_okay = 1; /* 1 == maybe */
	}

	/*
	 * Although we initialize opt->priv->paths with strdup_strings=0,
	 * that's just to avoid making yet another copy of an allocated
	 * string.  Putting the entry into paths means we are taking
	 * ownership, so we will later free it.
	 *
	 * In contrast, conflicted just has a subset of keys from paths, so
	 * we don't want to free those (it'd be a duplicate free).
	 */
	strmap_init_with_options(&opt->priv->paths, pool, 0);
	strmap_init_with_options(&opt->priv->conflicted, pool, 0);

	/*
	 * keys & strbufs in output will sometimes need to outlive "paths",
	 * so it will have a copy of relevant keys.  It's probably a small
	 * subset of the overall paths that have special output.
	 */
	strmap_init(&opt->priv->output);

	trace2_region_leave("merge", "allocate/init", opt->repo);
}


// Source: merge-ort.c
// Lines 4334-4458
