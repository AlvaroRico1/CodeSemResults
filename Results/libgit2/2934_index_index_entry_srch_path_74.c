static int index_entry_srch_path(const void *path, const void *array_member)
{
	const git_index_entry *entry = array_member;

	return strcmp((const char *)path, entry->path);
}


// Source: index.c
// Lines 203-208
