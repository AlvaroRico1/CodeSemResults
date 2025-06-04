int git_reference_foreach_name(
	git_repository *repo,
	git_reference_foreach_name_cb callback,
	void *payload)
{
	git_reference_iterator *iter;
	const char *refname;
	int error;

	if ((error = git_reference_iterator_new(&iter, repo)) < 0)
		return error;

	while (!(error = git_reference_next_name(&refname, iter))) {
		if ((error = callback(refname, payload)) != 0) {
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
// Lines 686-710
