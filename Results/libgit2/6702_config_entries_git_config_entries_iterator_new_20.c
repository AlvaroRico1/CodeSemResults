int git_config_entries_iterator_new(git_config_iterator **out, git_config_entries *entries)
{
	config_entries_iterator *it;

	it = git__calloc(1, sizeof(config_entries_iterator));
	GIT_ERROR_CHECK_ALLOC(it);
	it->parent.next = config_iterator_next;
	it->parent.free = config_iterator_free;
	it->head = entries->list;
	it->entries = entries;

	git_config_entries_incref(entries);
	*out = &it->parent;

	return 0;
}


// Source: config_entries.c
// Lines 222-237
