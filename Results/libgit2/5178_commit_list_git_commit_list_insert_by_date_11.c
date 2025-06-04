git_commit_list *git_commit_list_insert_by_date(git_commit_list_node *item, git_commit_list **list_p)
{
	git_commit_list **pp = list_p;
	git_commit_list *p;

	while ((p = *pp) != NULL) {
		if (git_commit_list_time_cmp(p->item, item) > 0)
			break;

		pp = &p->next;
	}

	return git_commit_list_insert(item, pp);
}


// Source: commit_list.c
// Lines 57-70
