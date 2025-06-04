static void assert_nth_gen_parent(unsigned int gen, const char *expected_oid)
{
	git_commit *parent = NULL;
	int error;
	
	error = git_commit_nth_gen_ancestor(&parent, commit, gen);

	if (expected_oid != NULL) {
		cl_assert_equal_i(0, error);
		cl_assert_equal_i(0, git_oid_streq(git_commit_id(parent), expected_oid));
	} else
		cl_assert_equal_i(GIT_ENOTFOUND, error);

	git_commit_free(parent);
}


// Source: parent.c
// Lines 25-39
