GIT_INLINE(int) index_find_in_entries(
	size_t *out, git_vector *entries, git_vector_cmp entry_srch,
	const char *path, size_t path_len, int stage)
{
	struct entry_srch_key srch_key;
	srch_key.path = path;
	srch_key.pathlen = !path_len ? strlen(path) : path_len;
	srch_key.stage = stage;
	return git_vector_bsearch2(out, entries, entry_srch, &srch_key);
}


// Source: index.c
// Lines 359-368
