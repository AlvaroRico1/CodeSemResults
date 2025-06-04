static void config_file_entry_free(git_config_entry *entry)
{
	git_config_entries *entries = (git_config_entries *) entry->payload;
	git_config_entries_free(entries);
}


// Source: config_file.c
// Lines 335-339
