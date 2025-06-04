static void create_index_names(struct checkout_name_entry *entries, size_t entries_len)
{
	size_t i;

	for (i = 0; i < entries_len; i++) {
		cl_git_pass(git_index_name_add(g_index,
			strlen(entries[i].ancestor) == 0 ? NULL : entries[i].ancestor,
			strlen(entries[i].ours) == 0 ? NULL : entries[i].ours,
			strlen(entries[i].theirs) == 0 ? NULL : entries[i].theirs));
	}
}


// Source: conflict.c
// Lines 116-126
