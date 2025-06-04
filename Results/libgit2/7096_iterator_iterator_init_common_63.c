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


// Source: iterator.c
// Lines 108-142
