int git_config_entries_dup(git_config_entries **out, git_config_entries *entries)
{
	git_config_entries *result = NULL;
	config_entry_list *head;
	int error;

	if ((error = git_config_entries_new(&result)) < 0)
		goto out;

	for (head = entries->list; head; head = head->next)
		if ((git_config_entries_dup_entry(result, head->entry)) < 0)
			goto out;

	*out = result;
	result = NULL;

out:
	git_config_entries_free(result);
	return error;
}


// Source: config_entries.c
// Lines 80-99
