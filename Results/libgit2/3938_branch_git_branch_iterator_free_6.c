void git_branch_iterator_free(git_branch_iterator *_iter)
{
	branch_iter *iter = (branch_iter *) _iter;

	if (iter == NULL)
		return;

	git_reference_iterator_free(iter->iter);
	git__free(iter);
}


// Source: branch.c
// Lines 276-285
