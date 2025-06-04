static void *iterate_refs(void *arg)
{
	struct th_data *data = (struct th_data *) arg;
	git_reference_iterator *i;
	git_reference *ref;
	int count = 0, error;
	git_repository *repo;

	cl_git_thread_pass(data, git_repository_open(&repo, data->path));
	do {
		error = git_reference_iterator_new(&i, repo);
	} while (error == GIT_ELOCKED);
	cl_git_thread_pass(data, error);

	for (count = 0; !git_reference_next(&ref, i); ++count) {
		cl_assert(ref != NULL);
		git_reference_free(ref);
	}

	if (g_expected > 0)
		cl_assert_equal_i(g_expected, count);

	git_reference_iterator_free(i);

	git_repository_free(repo);
	git_error_clear();
	return arg;
}


// Source: refdb.c
// Lines 36-63
