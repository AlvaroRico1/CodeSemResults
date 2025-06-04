static void index_existing_and_best(
	git_index_entry **existing,
	size_t *existing_position,
	git_index_entry **best,
	git_index *index,
	const git_index_entry *entry)
{
	git_index_entry *e;
	size_t pos;
	int error;

	error = index_find(&pos,
		index, entry->path, 0, GIT_INDEX_ENTRY_STAGE(entry));

	if (error == 0) {
		*existing = index->entries.contents[pos];
		*existing_position = pos;
		*best = index->entries.contents[pos];
		return;
	}

	*existing = NULL;
	*existing_position = 0;
	*best = NULL;

	if (GIT_INDEX_ENTRY_STAGE(entry) == 0) {
		for (; pos < index->entries.length; pos++) {
			int (*strcomp)(const char *a, const char *b) =
				index->ignore_case ? git__strcasecmp : git__strcmp;

			e = index->entries.contents[pos];

			if (strcomp(entry->path, e->path) != 0)
				break;

			if (GIT_INDEX_ENTRY_STAGE(e) == GIT_INDEX_STAGE_ANCESTOR) {
				*best = e;
				continue;
			} else {
				*best = e;
				break;
			}
		}
	}
}


// Source: index.c
// Lines 1280-1324
