void test_refs_iterator__list(void)
{
	git_reference_iterator *iter;
	git_vector output;
	git_reference *ref;

	cl_git_pass(git_vector_init(&output, 33, &refcmp_cb));
	cl_git_pass(git_reference_iterator_new(&iter, repo));

	while (1) {
		int error = git_reference_next(&ref, iter);
		if (error == GIT_ITEROVER)
			break;
		cl_git_pass(error);
		cl_git_pass(git_vector_insert(&output, ref));
	}

	git_reference_iterator_free(iter);

	assert_all_refnames_match(refnames, &output);
}


// Source: iterator.c
// Lines 100-120
