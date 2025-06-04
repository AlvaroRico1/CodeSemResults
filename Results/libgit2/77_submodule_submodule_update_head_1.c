static int submodule_update_head(git_submodule *submodule)
{
	git_tree *head = NULL;
	git_tree_entry *te = NULL;

	submodule->flags = submodule->flags &
		~(GIT_SUBMODULE_STATUS_IN_HEAD |
		  GIT_SUBMODULE_STATUS__HEAD_OID_VALID);

	/* if we can't look up file in current head, then done */
	if (git_repository_head_tree(&head, submodule->repo) < 0 ||
		git_tree_entry_bypath(&te, head, submodule->path) < 0)
		git_error_clear();
	else
		submodule_update_from_head_data(submodule, te->attr, git_tree_entry_id(te));

	git_tree_entry_free(te);
	git_tree_free(head);
	return 0;
}


// Source: submodule.c
// Lines 1649-1668
