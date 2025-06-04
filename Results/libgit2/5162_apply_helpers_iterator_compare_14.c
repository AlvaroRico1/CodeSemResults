static int iterator_compare(const git_index_entry *entry, void *_data)
{
	git_oid expected_id;

	struct iterator_compare_data *data = (struct iterator_compare_data *)_data;

	cl_assert_equal_i(GIT_INDEX_ENTRY_STAGE(entry), data->expected[data->idx].stage);
	cl_git_pass(git_oid_fromstr(&expected_id, data->expected[data->idx].oid_str));
	cl_assert_equal_oid(&entry->id, &expected_id);
	cl_assert_equal_i(entry->mode, data->expected[data->idx].mode);
	cl_assert_equal_s(entry->path, data->expected[data->idx].path);

	if (data->idx >= data->cnt)
		return -1;

	data->idx++;

	return 0;
}


// Source: apply_helpers.c
// Lines 10-28
