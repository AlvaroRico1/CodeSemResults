static int checkout_lookup_head_tree(git_tree **out, git_repository *repo)
{
	int error = 0;
	git_reference *ref = NULL;
	git_object *head;

	if (!(error = git_repository_head(&ref, repo)) &&
		!(error = git_reference_peel(&head, ref, GIT_OBJECT_TREE)))
		*out = (git_tree *)head;

	git_reference_free(ref);

	return error;
}


// Source: checkout.c
// Lines 1914-1927
