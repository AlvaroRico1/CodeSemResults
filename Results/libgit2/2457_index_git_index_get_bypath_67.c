const git_index_entry *git_index_get_bypath(
	git_index *index, const char *path, int stage)
{
	git_index_entry key = {{ 0 }};
	git_index_entry *value;

	GIT_ASSERT_ARG_WITH_RETVAL(index, NULL);

	key.path = path;
	GIT_INDEX_ENTRY_STAGE_SET(&key, stage);

	if (index->ignore_case)
		value = git_idxmap_icase_get((git_idxmap_icase *) index->entries_map, &key);
	else
		value = git_idxmap_get(index->entries_map, &key);

	if (!value) {
	    git_error_set(GIT_ERROR_INDEX, "index does not contain '%s'", path);
	    return NULL;
	}

	return value;
}


// Source: index.c
// Lines 859-881
