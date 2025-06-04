int git_merge_diff_list__find_renames(
	git_repository *repo,
	git_merge_diff_list *diff_list,
	const git_merge_options *opts)
{
	struct merge_diff_similarity *similarity_ours, *similarity_theirs;
	void **cache = NULL;
	size_t cache_size = 0;
	size_t src_count, tgt_count, i;
	int error = 0;

	GIT_ASSERT_ARG(diff_list);
	GIT_ASSERT_ARG(opts);

	if ((opts->flags & GIT_MERGE_FIND_RENAMES) == 0 ||
	    !diff_list->conflicts.length)
		return 0;

	similarity_ours = git__calloc(diff_list->conflicts.length,
		sizeof(struct merge_diff_similarity));
	GIT_ERROR_CHECK_ALLOC(similarity_ours);

	similarity_theirs = git__calloc(diff_list->conflicts.length,
		sizeof(struct merge_diff_similarity));
	GIT_ERROR_CHECK_ALLOC(similarity_theirs);

	/* Calculate similarity between items that were deleted from the ancestor
	 * and added in the other branch.
	 */
	if ((error = merge_diff_mark_similarity_exact(diff_list, similarity_ours, similarity_theirs)) < 0)
		goto done;

	if (opts->rename_threshold < 100 && diff_list->conflicts.length <= opts->target_limit) {
		GIT_ERROR_CHECK_ALLOC_MULTIPLY(&cache_size, diff_list->conflicts.length, 3);
		cache = git__calloc(cache_size, sizeof(void *));
		GIT_ERROR_CHECK_ALLOC(cache);

		merge_diff_list_count_candidates(diff_list, &src_count, &tgt_count);

		if (src_count > opts->target_limit || tgt_count > opts->target_limit) {
			/* TODO: report! */
		} else {
			if ((error = merge_diff_mark_similarity_inexact(
				repo, diff_list, similarity_ours, similarity_theirs, cache, opts)) < 0)
				goto done;
		}
	}

	/* For entries that are appropriately similar, merge the new name's entry
	 * into the old name.
	 */
	merge_diff_list_coalesce_renames(diff_list, similarity_ours, similarity_theirs, opts);

	/* And remove any entries that were merged and are now empty. */
	git_vector_remove_matching(&diff_list->conflicts, merge_diff_empty, NULL);

done:
	if (cache != NULL) {
		for (i = 0; i < cache_size; ++i) {
			if (cache[i] != NULL && cache[i] != &cache_invalid_marker)
				opts->metric->free_signature(cache[i], opts->metric->payload);
		}

		git__free(cache);
	}

	git__free(similarity_ours);
	git__free(similarity_theirs);

	return error;
}


// Source: merge.c
// Lines 1537-1607
