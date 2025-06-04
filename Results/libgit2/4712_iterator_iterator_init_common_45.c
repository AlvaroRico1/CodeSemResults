static int iterator_init_common(
	git_iterator *iter,
	git_repository *repo,
	git_index *index,
	git_iterator_options *given_opts)
{
	static git_iterator_options default_opts = GIT_ITERATOR_OPTIONS_INIT;
	git_iterator_options *options = given_opts ? given_opts : &default_opts;
	bool ignore_case;
	int precompose;
	int error;

	iter->repo = repo;
	iter->index = index;
	iter->flags = options->flags;

	if ((iter->flags & GIT_ITERATOR_IGNORE_CASE) != 0) {
		ignore_case = true;
	} else if ((iter->flags & GIT_ITERATOR_DONT_IGNORE_CASE) != 0) {
		ignore_case = false;
	} else if (repo) {
		git_index *index;

		if ((error = git_repository_index__weakptr(&index, iter->repo)) < 0)
			return error;

		ignore_case = !!index->ignore_case;

		if (ignore_case == 1)
			iter->flags |= GIT_ITERATOR_IGNORE_CASE;
		else
			iter->flags |= GIT_ITERATOR_DONT_IGNORE_CASE;
	} else {
		ignore_case = false;
	}

	/* try to look up precompose and set flag if appropriate */
	if (repo &&
		(iter->flags & GIT_ITERATOR_PRECOMPOSE_UNICODE) == 0 &&
		(iter->flags & GIT_ITERATOR_DONT_PRECOMPOSE_UNICODE) == 0) {

		if (git_repository__configmap_lookup(&precompose, repo, GIT_CONFIGMAP_PRECOMPOSE) < 0)
			git_error_clear();
		else if (precompose)
			iter->flags |= GIT_ITERATOR_PRECOMPOSE_UNICODE;
	}

	if ((iter->flags & GIT_ITERATOR_DONT_AUTOEXPAND))
		iter->flags |= GIT_ITERATOR_INCLUDE_TREES;

	if ((error = iterator_range_init(iter, options->start, options->end)) < 0 ||
		(error = iterator_pathlist_init(iter, &options->pathlist)) < 0)
		return error;

	iterator_set_ignore_case(iter, ignore_case);
	return 0;
}


// Source: iterator.c
// Lines 108-164
