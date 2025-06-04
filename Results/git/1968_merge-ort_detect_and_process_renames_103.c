static int detect_and_process_renames(struct merge_options *opt,
				      struct tree *merge_base,
				      struct tree *side1,
				      struct tree *side2)
{
	struct diff_queue_struct combined;
	struct rename_info *renames = &opt->priv->renames;
	int need_dir_renames, s, clean = 1;
	unsigned detection_run = 0;

	memset(&combined, 0, sizeof(combined));
	if (!possible_renames(renames))
		goto cleanup;

	trace2_region_enter("merge", "regular renames", opt->repo);
	detection_run |= detect_regular_renames(opt, MERGE_SIDE1);
	detection_run |= detect_regular_renames(opt, MERGE_SIDE2);
	if (renames->redo_after_renames && detection_run) {
		int i, side;
		struct diff_filepair *p;

		/* Cache the renames, we found */
		for (side = MERGE_SIDE1; side <= MERGE_SIDE2; side++) {
			for (i = 0; i < renames->pairs[side].nr; ++i) {
				p = renames->pairs[side].queue[i];
				possibly_cache_new_pair(renames, p, side, NULL);
			}
		}

		/* Restart the merge with the cached renames */
		renames->redo_after_renames = 2;
		trace2_region_leave("merge", "regular renames", opt->repo);
		goto cleanup;
	}
	use_cached_pairs(opt, &renames->cached_pairs[1], &renames->pairs[1]);
	use_cached_pairs(opt, &renames->cached_pairs[2], &renames->pairs[2]);
	trace2_region_leave("merge", "regular renames", opt->repo);

	trace2_region_enter("merge", "directory renames", opt->repo);
	need_dir_renames =
	  !opt->priv->call_depth &&
	  (opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_TRUE ||
	   opt->detect_directory_renames == MERGE_DIRECTORY_RENAMES_CONFLICT);

	if (need_dir_renames) {
		get_provisional_directory_renames(opt, MERGE_SIDE1, &clean);
		get_provisional_directory_renames(opt, MERGE_SIDE2, &clean);
		handle_directory_level_conflicts(opt);
	}

	ALLOC_GROW(combined.queue,
		   renames->pairs[1].nr + renames->pairs[2].nr,
		   combined.alloc);
	clean &= collect_renames(opt, &combined, MERGE_SIDE1,
				 &renames->dir_renames[2],
				 &renames->dir_renames[1]);
	clean &= collect_renames(opt, &combined, MERGE_SIDE2,
				 &renames->dir_renames[1],
				 &renames->dir_renames[2]);
	STABLE_QSORT(combined.queue, combined.nr, compare_pairs);
	trace2_region_leave("merge", "directory renames", opt->repo);

	trace2_region_enter("merge", "process renames", opt->repo);
	clean &= process_renames(opt, &combined);
	trace2_region_leave("merge", "process renames", opt->repo);

	goto simple_cleanup; /* collect_renames() handles some of cleanup */

cleanup:
	/*
	 * Free now unneeded filepairs, which would have been handled
	 * in collect_renames() normally but we skipped that code.
	 */
	for (s = MERGE_SIDE1; s <= MERGE_SIDE2; s++) {
		struct diff_queue_struct *side_pairs;
		int i;

		side_pairs = &renames->pairs[s];
		for (i = 0; i < side_pairs->nr; ++i) {
			struct diff_filepair *p = side_pairs->queue[i];
			pool_diff_free_filepair(&opt->priv->pool, p);
		}
	}

simple_cleanup:
	/* Free memory for renames->pairs[] and combined */
	for (s = MERGE_SIDE1; s <= MERGE_SIDE2; s++) {
		free(renames->pairs[s].queue);
		DIFF_QUEUE_CLEAR(&renames->pairs[s]);
	}
	if (combined.nr) {
		int i;
		for (i = 0; i < combined.nr; i++)
			pool_diff_free_filepair(&opt->priv->pool,
						combined.queue[i]);
		free(combined.queue);
	}

	return clean;
}


// Source: merge-ort.c
// Lines 3056-3155
