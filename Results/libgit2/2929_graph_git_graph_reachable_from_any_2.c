int git_graph_reachable_from_any(
		git_repository *repo,
		const git_oid *commit_id,
		const git_oid descendant_array[],
		size_t length)
{
	git_revwalk *walk = NULL;
	git_vector list;
	git_commit_list *result = NULL;
	git_commit_list_node *commit;
	size_t i;
	uint32_t minimum_generation = 0xffffffff;
	int error = 0;

	if (!length)
		return 0;

	for (i = 0; i < length; ++i) {
		if (git_oid_equal(commit_id, &descendant_array[i]))
			return 1;
	}

	if ((error = git_vector_init(&list, length + 1, NULL)) < 0)
		return error;

	if ((error = git_revwalk_new(&walk, repo)) < 0)
		goto done;

	for (i = 0; i < length; i++) {
		commit = git_revwalk__commit_lookup(walk, &descendant_array[i]);
		if (commit == NULL) {
			error = -1;
			goto done;
		}

		git_vector_insert(&list, commit);
		if (minimum_generation > commit->generation)
			minimum_generation = commit->generation;
	}

	commit = git_revwalk__commit_lookup(walk, commit_id);
	if (commit == NULL) {
		error = -1;
		goto done;
	}

	if (minimum_generation > commit->generation)
		minimum_generation = commit->generation;

	if ((error = git_merge__bases_many(&result, walk, commit, &list, minimum_generation)) < 0)
		goto done;

	if (result) {
		error = git_oid_equal(commit_id, &result->item->oid);
	} else {
		/* No merge-base found, it's not a descendant */
		error = 0;
	}

done:
	git_commit_list_free(&result);
	git_vector_free(&list);
	git_revwalk_free(walk);
	return error;
}


// Source: graph.c
// Lines 185-249
