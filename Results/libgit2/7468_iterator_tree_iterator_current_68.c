static int tree_iterator_current(
	const git_index_entry **out, git_iterator *i)
{
	tree_iterator *iter = (tree_iterator *)i;

	if (!iterator__has_been_accessed(i))
		return iter->base.cb->advance(out, i);

	if (!iter->frames.size) {
		*out = NULL;
		return GIT_ITEROVER;
	}

	*out = &iter->entry;
	return 0;
}


// Source: iterator.c
// Lines 713-728
