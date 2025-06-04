const git_blame_hunk *git_blame_get_hunk_byline(git_blame *blame, size_t lineno)
{
	size_t i, new_lineno = lineno;

	GIT_ASSERT_ARG_WITH_RETVAL(blame, NULL);

	if (!git_vector_bsearch2(&i, &blame->hunks, hunk_byfinalline_search_cmp, &new_lineno)) {
		return git_blame_get_hunk_byindex(blame, (uint32_t)i);
	}

	return NULL;
}


// Source: blame.c
// Lines 189-200
