int git_index_iterator_new(
	git_index_iterator **iterator_out,
	git_index *index)
{
	git_index_iterator *it;
	int error;

	GIT_ASSERT_ARG(iterator_out);
	GIT_ASSERT_ARG(index);

	it = git__calloc(1, sizeof(git_index_iterator));
	GIT_ERROR_CHECK_ALLOC(it);

	if ((error = git_index_snapshot_new(&it->snap, index)) < 0) {
		git__free(it);
		return error;
	}

	it->index = index;

	*iterator_out = it;
	return 0;
}


// Source: index.c
// Lines 2007-2029
