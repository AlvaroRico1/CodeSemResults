void validate_workdir_unchanged(git_repository *repo)
{
	git_tree *head;
	git_index *index;
	git_iterator *head_iterator, *workdir_iterator, *iterators[2];
	git_iterator_options workdir_opts = GIT_ITERATOR_OPTIONS_INIT;

	cl_git_pass(git_repository_head_tree(&head, repo));
	cl_git_pass(git_repository_index(&index, repo));

	workdir_opts.flags |= GIT_ITERATOR_INCLUDE_HASH;

	cl_git_pass(git_iterator_for_tree(&head_iterator, head, NULL));
	cl_git_pass(git_iterator_for_workdir(&workdir_iterator, repo, index, NULL, &workdir_opts));

	iterators[0] = head_iterator;
	iterators[1] = workdir_iterator;

	cl_git_pass(git_iterator_walk(iterators, 2, iterator_eq, NULL));

	git_iterator_free(head_iterator);
	git_iterator_free(workdir_iterator);

	git_tree_free(head);
	git_index_free(index);
}


// Source: apply_helpers.c
// Lines 110-135
