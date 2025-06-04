static int checkout_conflicts_coalesce_renames(
	checkout_data *data)
{
	git_index *index;
	const git_index_name_entry *name_entry;
	checkout_conflictdata *ancestor_conflict, *our_conflict, *their_conflict;
	size_t i, names;
	int error = 0;

	if ((index = git_iterator_index(data->target)) == NULL)
		return 0;

	/* Juggle entries based on renames */
	names = git_index_name_entrycount(index);

	for (i = 0; i < names; i++) {
		name_entry = git_index_name_get_byindex(index, i);

		if ((error = checkout_conflicts_load_byname_entry(
			&ancestor_conflict, &our_conflict, &their_conflict,
			data, name_entry)) < 0)
			goto done;

		if (our_conflict && our_conflict != ancestor_conflict) {
			ancestor_conflict->ours = our_conflict->ours;
			our_conflict->ours = NULL;

			if (our_conflict->theirs)
				our_conflict->name_collision = 1;

			if (our_conflict->name_collision)
				ancestor_conflict->name_collision = 1;
		}

		if (their_conflict && their_conflict != ancestor_conflict) {
			ancestor_conflict->theirs = their_conflict->theirs;
			their_conflict->theirs = NULL;

			if (their_conflict->ours)
				their_conflict->name_collision = 1;

			if (their_conflict->name_collision)
				ancestor_conflict->name_collision = 1;
		}

		if (our_conflict && our_conflict != ancestor_conflict &&
			their_conflict && their_conflict != ancestor_conflict)
			ancestor_conflict->one_to_two = 1;
	}

	git_vector_remove_matching(
		&data->update_conflicts, checkout_conflictdata_empty, NULL);

done:
	return error;
}


// Source: checkout.c
// Lines 1108-1163
