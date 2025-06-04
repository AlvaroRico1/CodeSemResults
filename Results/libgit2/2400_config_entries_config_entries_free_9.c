static void config_entries_free(git_config_entries *entries)
{
	config_entry_list *list = NULL, *next;
	config_entry_map_head *head;

	git_strmap_foreach_value(entries->map, head,
		git__free((char *) head->entry->name); git__free(head)
	);
	git_strmap_free(entries->map);

	list = entries->list;
	while (list != NULL) {
		next = list->next;
		git__free((char *) list->entry->value);
		git__free(list->entry);
		git__free(list);
		list = next;
	}

	git__free(entries);
}


// Source: config_entries.c
// Lines 106-126
