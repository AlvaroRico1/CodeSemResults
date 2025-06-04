int git_repository_head_tree(git_tree **tree, git_repository *repo)
{
	git_reference *head;
	git_object *obj;
	int error;

	if ((error = git_repository_head(&head, repo)) < 0)
		return error;

	if ((error = git_reference_peel(&obj, head, GIT_OBJECT_TREE)) < 0)
		goto cleanup;

	*tree = (git_tree *)obj;

cleanup:
	git_reference_free(head);
	return error;
}


// Source: repository.c
// Lines 2765-2782
