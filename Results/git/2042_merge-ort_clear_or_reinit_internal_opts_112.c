static void clear_or_reinit_internal_opts(struct merge_options_internal *opti,
					  int reinitialize)
{
	struct rename_info *renames = &opti->renames;
	int i;
	void (*strmap_clear_func)(struct strmap *, int) =
		reinitialize ? strmap_partial_clear : strmap_clear;
	void (*strintmap_clear_func)(struct strintmap *) =
		reinitialize ? strintmap_partial_clear : strintmap_clear;
	void (*strset_clear_func)(struct strset *) =
		reinitialize ? strset_partial_clear : strset_clear;

	strmap_clear_func(&opti->paths, 0);

	/*
	 * All keys and values in opti->conflicted are a subset of those in
	 * opti->paths.  We don't want to deallocate anything twice, so we
	 * don't free the keys and we pass 0 for free_values.
	 */
	strmap_clear_func(&opti->conflicted, 0);

	if (opti->attr_index.cache_nr) /* true iff opt->renormalize */
		discard_index(&opti->attr_index);

	/* Free memory used by various renames maps */
	for (i = MERGE_SIDE1; i <= MERGE_SIDE2; ++i) {
		strintmap_clear_func(&renames->dirs_removed[i]);
		strmap_clear_func(&renames->dir_renames[i], 0);
		strintmap_clear_func(&renames->relevant_sources[i]);
		if (!reinitialize)
			assert(renames->cached_pairs_valid_side == 0);
		if (i != renames->cached_pairs_valid_side &&
		    -1 != renames->cached_pairs_valid_side) {
			strset_clear_func(&renames->cached_target_names[i]);
			strmap_clear_func(&renames->cached_pairs[i], 1);
			strset_clear_func(&renames->cached_irrelevant[i]);
			partial_clear_dir_rename_count(&renames->dir_rename_count[i]);
			if (!reinitialize)
				strmap_clear(&renames->dir_rename_count[i], 1);
		}
	}
	for (i = MERGE_SIDE1; i <= MERGE_SIDE2; ++i) {
		strintmap_clear_func(&renames->deferred[i].possible_trivial_merges);
		strset_clear_func(&renames->deferred[i].target_dirs);
		renames->deferred[i].trivial_merges_okay = 1; /* 1 == maybe */
	}
	renames->cached_pairs_valid_side = 0;
	renames->dir_rename_mask = 0;

	if (!reinitialize) {
		struct hashmap_iter iter;
		struct strmap_entry *e;

		/* Release and free each strbuf found in output */
		strmap_for_each_entry(&opti->output, &iter, e) {
			struct strbuf *sb = e->value;
			strbuf_release(sb);
			/*
			 * While strictly speaking we don't need to free(sb)
			 * here because we could pass free_values=1 when
			 * calling strmap_clear() on opti->output, that would
			 * require strmap_clear to do another
			 * strmap_for_each_entry() loop, so we just free it
			 * while we're iterating anyway.
			 */
			free(sb);
		}


// Source: merge-ort.c
// Lines 516-582
