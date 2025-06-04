void test_refs_iterator__concurrent_delete(void)
{
	git_reference_iterator *iter;
	size_t full_count = 0, concurrent_count = 0;
	const char *name;
	int error;

	cl_git_sandbox_cleanup();
	repo = cl_git_sandbox_init("testrepo");

	cl_git_pass(git_reference_iterator_new(&iter, repo));
	while ((error = git_reference_next_name(&name, iter)) == 0) {
		full_count++;
	}

	git_reference_iterator_free(iter);
	cl_assert_equal_i(GIT_ITEROVER, error);

	cl_git_pass(git_reference_iterator_new(&iter, repo));
	while ((error = git_reference_next_name(&name, iter)) == 0) {
		cl_git_pass(git_reference_remove(repo, name));
		concurrent_count++;
	}

	git_reference_iterator_free(iter);
	cl_assert_equal_i(GIT_ITEROVER, error);

	cl_assert_equal_i(full_count, concurrent_count);
}


// Source: iterator.c
// Lines 246-274
