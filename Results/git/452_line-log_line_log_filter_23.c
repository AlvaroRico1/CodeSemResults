int line_log_filter(struct rev_info *rev)
{
	struct commit *commit;
	struct commit_list *list = rev->commits;
	struct commit_list *out = NULL, **pp = &out;

	while (list) {
		struct commit_list *to_free = NULL;
		commit = list->item;
		if (line_log_process_ranges_arbitrary_commit(rev, commit)) {
			*pp = list;
			pp = &list->next;
		} else
			to_free = list;
		list = list->next;
		free(to_free);
	}


// Source: line-log.c
// Lines 1301-1317
