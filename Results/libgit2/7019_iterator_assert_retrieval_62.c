static void assert_retrieval(unsigned int flags, unsigned int expected_count)
{
	git_branch_iterator *iter;
	git_reference *ref;
	int count = 0, error;
	git_branch_t type;

	cl_git_pass(git_branch_iterator_new(&iter, repo, flags));
	while ((error = git_branch_next(&ref, &type, iter)) == 0) {
		count++;
		git_reference_free(ref);
	}

	git_branch_iterator_free(iter);
	cl_assert_equal_i(error, GIT_ITEROVER);
	cl_assert_equal_i(expected_count, count);
}


// Source: iterator.c
// Lines 31-47
