__KHASH_TYPE(idx, const git_index_entry *, git_index_entry *)
__KHASH_TYPE(idxicase, const git_index_entry *, git_index_entry *)

/* This is __ac_X31_hash_string but with tolower and it takes the entry's stage into account */
static kh_inline khint_t idxentry_hash(const git_index_entry *e)
{
	const char *s = e->path;
	khint_t h = (khint_t)git__tolower(*s);
	if (h) for (++s ; *s; ++s) h = (h << 5) - h + (khint_t)git__tolower(*s);
	return h + GIT_INDEX_ENTRY_STAGE(e);
}


// Source: idxmap.c
// Lines 17-27
