static int iterator_advance(
	const git_index_entry **entry,
	git_iterator *iterator)
{
	const git_index_entry *prev_entry = *entry;
	int cmp, error;

	/* if we're looking for conflicts, we only want to report
	 * one conflict for each file, instead of all three sides.
	 * so if this entry is a conflict for this file, and the
	 * previous one was a conflict for the same file, skip it.
	 */
	while ((error = git_iterator_advance(entry, iterator)) == 0) {
		if (!(iterator->flags & GIT_ITERATOR_INCLUDE_CONFLICTS) ||
			!git_index_entry_is_conflict(prev_entry) ||
			!git_index_entry_is_conflict(*entry))
			break;

		cmp = (iterator->flags & GIT_ITERATOR_IGNORE_CASE) ?
			strcasecmp(prev_entry->path, (*entry)->path) :
			strcmp(prev_entry->path, (*entry)->path);

		if (cmp)
			break;
	}

	if (error == GIT_ITEROVER) {
		*entry = NULL;
		error = 0;
	}

	return error;
}


// Source: diff_generate.c
// Lines 956-988
