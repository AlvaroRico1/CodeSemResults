static int write_entries(git_index *index, git_filebuf *file)
{
	int error = 0;
	size_t i;
	git_vector case_sorted = GIT_VECTOR_INIT, *entries = NULL;
	git_index_entry *entry;
	const char *last = NULL;

	/* If index->entries is sorted case-insensitively, then we need
	 * to re-sort it case-sensitively before writing */
	if (index->ignore_case) {
		if ((error = git_vector_dup(&case_sorted, &index->entries, git_index_entry_cmp)) < 0)
			goto done;

		git_vector_sort(&case_sorted);
		entries = &case_sorted;
	} else {
		entries = &index->entries;
	}

	if (index->version >= INDEX_VERSION_NUMBER_COMP)
		last = "";

	git_vector_foreach(entries, i, entry) {
		if ((error = write_disk_entry(file, entry, last)) < 0)
			break;
		if (index->version >= INDEX_VERSION_NUMBER_COMP)
			last = entry->path;
	}

done:
	git_vector_free(&case_sorted);
	return error;
}


// Source: index.c
// Lines 2854-2887
