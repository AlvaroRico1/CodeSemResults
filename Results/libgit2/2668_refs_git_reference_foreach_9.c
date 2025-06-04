int git_reference_foreach(
	git_repository *repo,
	git_reference_foreach_cb callback,
	void *payload)
{
	git_reference_iterator *iter;
	git_reference *ref;
	int error;

	if ((error = git_reference_iterator_new(&iter, repo)) < 0)
		return error;

	while (!(error = git_reference_next(&ref, iter))) {
		if ((error = callback(ref, payload)) != 0) {
			git_error_set_after_callback(error);
			break;
		}
	}

	if (error == GIT_ITEROVER)
		error = 0;

	git_reference_iterator_free(iter);
	return error;
}


// Source: refs.c
// Lines 660-684
