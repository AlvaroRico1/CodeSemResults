static int index_entry_isrch_path(const void *path, const void *array_member)
{
	const git_index_entry *entry = array_member;

	return strcasecmp((const char *)path, entry->path);
}


// Source: index.c
// Lines 210-215
