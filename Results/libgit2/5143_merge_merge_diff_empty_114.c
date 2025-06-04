static int merge_diff_empty(const git_vector *conflicts, size_t idx, void *p)
{
	git_merge_diff *conflict = conflicts->contents[idx];

	GIT_UNUSED(p);

	return (!GIT_MERGE_INDEX_ENTRY_EXISTS(conflict->ancestor_entry) &&
		!GIT_MERGE_INDEX_ENTRY_EXISTS(conflict->our_entry) &&
		!GIT_MERGE_INDEX_ENTRY_EXISTS(conflict->their_entry));
}


// Source: merge.c
// Lines 1505-1514
