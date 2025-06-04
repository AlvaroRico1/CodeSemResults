static int reuc_srch(const void *key, const void *array_member)
{
	const git_index_reuc_entry *reuc = array_member;

	return strcmp(key, reuc->path);
}


// Source: index.c
// Lines 290-295
