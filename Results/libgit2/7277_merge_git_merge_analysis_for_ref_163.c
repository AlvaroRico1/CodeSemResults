int git_merge_analysis_for_ref(
	git_merge_analysis_t *analysis_out,
	git_merge_preference_t *preference_out,
	git_repository *repo,
	git_reference *our_ref,
	const git_annotated_commit **their_heads,
	size_t their_heads_len)
{
	git_annotated_commit *ancestor_head = NULL, *our_head = NULL;
	int error = 0;
	bool unborn;

	GIT_ASSERT_ARG(analysis_out);
	GIT_ASSERT_ARG(preference_out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(their_heads && their_heads_len > 0);

	if (their_heads_len != 1) {
		git_error_set(GIT_ERROR_MERGE, "can only merge a single branch");
		error = -1;
		goto done;
	}

	*analysis_out = GIT_MERGE_ANALYSIS_NONE;

	if ((error = merge_preference(preference_out, repo)) < 0)
		goto done;

	if ((error = git_reference__is_unborn_head(&unborn, our_ref, repo)) < 0)
		goto done;

	if (unborn) {
		*analysis_out |= GIT_MERGE_ANALYSIS_FASTFORWARD | GIT_MERGE_ANALYSIS_UNBORN;
		error = 0;
		goto done;
	}

	if ((error = merge_heads(&ancestor_head, &our_head, repo, our_ref, their_heads, their_heads_len)) < 0)
		goto done;

	/* We're up-to-date if we're trying to merge our own common ancestor. */
	if (ancestor_head && git_oid_equal(
		git_annotated_commit_id(ancestor_head), git_annotated_commit_id(their_heads[0])))
		*analysis_out |= GIT_MERGE_ANALYSIS_UP_TO_DATE;

	/* We're fastforwardable if we're our own common ancestor. */
	else if (ancestor_head && git_oid_equal(
		git_annotated_commit_id(ancestor_head), git_annotated_commit_id(our_head)))
		*analysis_out |= GIT_MERGE_ANALYSIS_FASTFORWARD | GIT_MERGE_ANALYSIS_NORMAL;

	/* Otherwise, just a normal merge is possible. */
	else
		*analysis_out |= GIT_MERGE_ANALYSIS_NORMAL;

done:
	git_annotated_commit_free(ancestor_head);
	git_annotated_commit_free(our_head);
	return error;
}


// Source: merge.c
// Lines 3235-3293
