void git_commit_list_free(git_commit_list **list_p)
{
	git_commit_list *list = *list_p;

	if (list == NULL)
		return;

	while (list) {
		git_commit_list *temp = list;
		list = temp->next;
		git__free(temp);
	}


// Source: commit_list.c
// Lines 92-103
