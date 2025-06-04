static void config_iterator_free(git_config_iterator *iter)
{
	config_entries_iterator *it = (config_entries_iterator *) iter;
	git_config_entries_free(it->entries);
	git__free(it);
}


// Source: config_entries.c
// Lines 200-205
