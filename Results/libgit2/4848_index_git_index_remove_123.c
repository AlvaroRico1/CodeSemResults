int git_index_remove(git_index *index, const char *path, int stage)
{
	int error;
	size_t position;
	git_index_entry remove_key = {{ 0 }};

	remove_key.path = path;
	GIT_INDEX_ENTRY_STAGE_SET(&remove_key, stage);

	index_map_delete(index->entries_map, &remove_key, index->ignore_case);

	if (index_find(&position, index, path, 0, stage) < 0) {
		git_error_set(
			GIT_ERROR_INDEX, "index does not contain %s at stage %d", path, stage);
		error = GIT_ENOTFOUND;
	} else {
		error = index_remove_entry(index, position);
	}

	return error;
}


// Source: index.c
// Lines 1694-1714
