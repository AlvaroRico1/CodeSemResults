int git_checkout_index(
	git_repository *repo,
	git_index *index,
	const git_checkout_options *opts)
{
	git_iterator_options iter_opts = GIT_ITERATOR_OPTIONS_INIT;
	int error, owned = 0;
	git_iterator *index_i;

	if (!index && !repo) {
		git_error_set(GIT_ERROR_CHECKOUT,
			"must provide either repository or index to checkout");
		return -1;
	}

	if (index && repo &&
		git_index_owner(index) &&
		git_index_owner(index) != repo) {
		git_error_set(GIT_ERROR_CHECKOUT,
			"index to checkout does not match repository");
		return -1;
	} else if(index && repo && !git_index_owner(index)) {
		GIT_REFCOUNT_OWN(index, repo);
		owned = 1;
	}

	if (!repo)
		repo = git_index_owner(index);

	if (!index && (error = git_repository_index__weakptr(&index, repo)) < 0)
		return error;
	GIT_REFCOUNT_INC(index);

	setup_pathspecs(&iter_opts, opts);

	if (!(error = git_iterator_for_index(&index_i, repo, index, &iter_opts)))
		error = git_checkout_iterator(index_i, index, opts);

	if (owned)
		GIT_REFCOUNT_OWN(index, NULL);

	git_iterator_free(index_i);
	git_index_free(index);

	return error;
}


// Source: checkout.c
// Lines 2688-2733
