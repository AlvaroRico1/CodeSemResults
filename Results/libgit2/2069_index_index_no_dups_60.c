static int index_no_dups(void **old, void *new)
{
	const git_index_entry *entry = new;
	GIT_UNUSED(old);
	git_error_set(GIT_ERROR_INDEX, "'%s' appears multiple times at stage %d",
		entry->path, GIT_INDEX_ENTRY_STAGE(entry));
	return GIT_EEXISTS;
}


// Source: index.c
// Lines 1271-1278
