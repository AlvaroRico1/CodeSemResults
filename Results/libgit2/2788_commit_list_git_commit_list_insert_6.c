git_commit_list *git_commit_list_insert(git_commit_list_node *item, git_commit_list **list_p)
{
	git_commit_list *new_list = git__malloc(sizeof(git_commit_list));
	if (new_list != NULL) {
		new_list->item = item;
		new_list->next = *list_p;
	}
	*list_p = new_list;
	return new_list;
}


// Source: commit_list.c
// Lines 46-55
