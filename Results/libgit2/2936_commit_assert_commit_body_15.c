static void assert_commit_body(const char *expected, const char *given)
{
	git_commit *dummy;

	cl_assert(dummy = git__calloc(1, sizeof(struct git_commit)));

	dummy->raw_message = git__strdup(given);
	cl_assert_equal_s(expected, git_commit_body(dummy));

	git_commit__free(dummy);
}


// Source: commit.c
// Lines 126-136
