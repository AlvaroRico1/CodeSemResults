static int checkout_conflicts_foreach(
	checkout_data *data,
	git_index *index,
	git_iterator *workdir,
	git_vector *pathspec,
	int (*cb)(const git_index_entry *, const git_index_entry *, const git_index_entry *, void *),
	void *payload)
{
	git_index_conflict_iterator *iterator = NULL;
	const git_index_entry *ancestor, *ours, *theirs;
	int error = 0;

	if ((error = git_index_conflict_iterator_new(&iterator, index)) < 0)
		goto done;

	/* Collect the conflicts */
	while ((error = git_index_conflict_next(&ancestor, &ours, &theirs, iterator)) == 0) {
		if (!conflict_pathspec_match(data, workdir, pathspec, ancestor, ours, theirs))
			continue;

		if ((error = cb(ancestor, ours, theirs, payload)) < 0)
			goto done;
	}

	if (error == GIT_ITEROVER)
		error = 0;

done:
	git_index_conflict_iterator_free(iterator);

	return error;
}


// Source: checkout.c
// Lines 929-960
