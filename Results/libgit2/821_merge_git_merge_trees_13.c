int git_merge_trees(
	git_index **out,
	git_repository *repo,
	const git_tree *ancestor_tree,
	const git_tree *our_tree,
	const git_tree *their_tree,
	const git_merge_options *merge_opts)
{
	git_iterator *ancestor_iter = NULL, *our_iter = NULL, *their_iter = NULL;
	git_iterator_options iter_opts = GIT_ITERATOR_OPTIONS_INIT;
	int error;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);

	/* if one side is treesame to the ancestor, take the other side */
	if (ancestor_tree && merge_opts && (merge_opts->flags & GIT_MERGE_SKIP_REUC)) {
		const git_tree *result = NULL;
		const git_oid *ancestor_tree_id = git_tree_id(ancestor_tree);

		if (our_tree && !git_oid_cmp(ancestor_tree_id, git_tree_id(our_tree)))
			result = their_tree;
		else if (their_tree && !git_oid_cmp(ancestor_tree_id, git_tree_id(their_tree)))
			result = our_tree;

		if (result) {
			if ((error = git_index_new(out)) == 0)
    			error = git_index_read_tree(*out, result);

			return error;
		}
	}


// Source: merge.c
// Lines 2177-2208
