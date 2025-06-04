git_commit_list_node *git_commit_list_pop(git_commit_list **stack)
{
	git_commit_list *top = *stack;
	git_commit_list_node *item = top ? top->item : NULL;

	if (top) {
		*stack = top->next;
		git__free(top);
	}
	return item;
}


// Source: commit_list.c
// Lines 108-118
