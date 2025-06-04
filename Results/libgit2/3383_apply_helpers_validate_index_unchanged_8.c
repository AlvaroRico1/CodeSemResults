void validate_index_unchanged(git_repository *repo)
{
	git_tree *head;
	git_index *index;
	git_iterator *head_iterator, *index_iterator, *iterators[2];

	cl_git_pass(git_repository_head_tree(&head, repo));
	cl_git_pass(git_repository_index(&index, repo));

	cl_git_pass(git_iterator_for_tree(&head_iterator, head, NULL));
	cl_git_pass(git_iterator_for_index(&index_iterator, repo, index, NULL));

	iterators[0] = head_iterator;
	iterators[1] = index_iterator;

	cl_git_pass(git_iterator_walk(iterators, 2, iterator_eq, NULL));

	git_iterator_free(head_iterator);
	git_iterator_free(index_iterator);

	git_tree_free(head);
	git_index_free(index);
}


// Source: apply_helpers.c
// Lines 86-108
