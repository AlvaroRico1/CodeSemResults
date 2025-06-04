int git_branch_iterator_new(
	git_branch_iterator **out,
	git_repository *repo,
	git_branch_t list_flags)
{
	branch_iter *iter;

	iter = git__calloc(1, sizeof(branch_iter));
	GIT_ERROR_CHECK_ALLOC(iter);

	iter->flags = list_flags;

	if (git_reference_iterator_new(&iter->iter, repo) < 0) {
		git__free(iter);
		return -1;
	}

	*out = (git_branch_iterator *) iter;

	return 0;
}


// Source: branch.c
// Lines 254-274
