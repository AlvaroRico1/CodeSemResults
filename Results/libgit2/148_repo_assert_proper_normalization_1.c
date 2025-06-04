static void assert_proper_normalization(git_index *index, const char *filename, const char *expected_sha)
{
	size_t index_pos;
	const git_index_entry *entry;

	add_to_workdir(filename, CONTENT);
	cl_git_pass(git_index_add_bypath(index, filename));

	cl_assert(!git_index_find(&index_pos, index, filename));

	entry = git_index_get_byindex(index, index_pos);
	cl_assert_equal_i(0, git_oid_streq(&entry->id, expected_sha));
}


// Source: repo.c
// Lines 241-253
