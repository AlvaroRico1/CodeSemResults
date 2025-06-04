static int create_virtual_base(
	git_annotated_commit **out,
	git_repository *repo,
	git_annotated_commit *one,
	git_annotated_commit *two,
	const git_merge_options *opts,
	size_t recursion_level)
{
	git_annotated_commit *result = NULL;
	git_index *index = NULL;
	git_merge_options virtual_opts = GIT_MERGE_OPTIONS_INIT;

	/* Conflicts in the merge base creation do not propagate to conflicts
	 * in the result; the conflicted base will act as the common ancestor.
	 */
	if (opts)
		memcpy(&virtual_opts, opts, sizeof(git_merge_options));

	virtual_opts.flags &= ~GIT_MERGE_FAIL_ON_CONFLICT;
	virtual_opts.flags |= GIT_MERGE_VIRTUAL_BASE;

	if ((merge_annotated_commits(&index, NULL, repo, one, two,
			recursion_level + 1, &virtual_opts)) < 0)
		return -1;

	result = git__calloc(1, sizeof(git_annotated_commit));
	GIT_ERROR_CHECK_ALLOC(result);
	result->type = GIT_ANNOTATED_COMMIT_VIRTUAL;
	result->index = index;

	if (insert_head_ids(&result->parents, one) < 0 ||
		insert_head_ids(&result->parents, two) < 0) {
		git_annotated_commit_free(result);
		return -1;
	}

	*out = result;
	return 0;
}


// Source: merge.c
// Lines 2264-2302
