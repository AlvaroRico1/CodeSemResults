static int index_iterator_current(
	const git_index_entry **out, git_iterator *i)
{
	index_iterator *iter = (index_iterator *)i;

	if (!iterator__has_been_accessed(i))
		return iter->base.cb->advance(out, i);

	if (iter->entry == NULL) {
		*out = NULL;
		return GIT_ITEROVER;
	}

	*out = iter->entry;
	return 0;
}


// Source: iterator.c
// Lines 2041-2056
