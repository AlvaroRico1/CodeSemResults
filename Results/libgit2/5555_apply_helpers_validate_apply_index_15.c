void validate_apply_index(
	git_repository *repo,
	struct merge_index_entry *index_entries,
	size_t index_cnt)
{
	git_index *index;
	git_iterator *iterator;
	struct iterator_compare_data data = { index_entries, index_cnt };

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_iterator_for_index(&iterator, repo, index, NULL));

	cl_git_pass(git_iterator_foreach(iterator, iterator_compare, &data));
	cl_assert_equal_i(data.idx, data.cnt);

	git_iterator_free(iterator);
	git_index_free(index);
}


// Source: apply_helpers.c
// Lines 52-69
