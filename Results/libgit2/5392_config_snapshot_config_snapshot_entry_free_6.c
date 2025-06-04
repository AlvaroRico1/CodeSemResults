static void config_snapshot_entry_free(git_config_entry *entry)
{
	git_config_entries *entries = (git_config_entries *) entry->payload;
	git_config_entries_free(entries);
}


// Source: config_snapshot.c
// Lines 45-49
