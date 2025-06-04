int git_index_conflict_iterator_new(
	git_index_conflict_iterator **iterator_out,
	git_index *index)
{
	git_index_conflict_iterator *it = NULL;

	GIT_ASSERT_ARG(iterator_out);
	GIT_ASSERT_ARG(index);

	it = git__calloc(1, sizeof(git_index_conflict_iterator));
	GIT_ERROR_CHECK_ALLOC(it);

	it->index = index;

	*iterator_out = it;
	return 0;
}


// Source: index.c
// Lines 2054-2070
