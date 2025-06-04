static int count_cfg_entries(const git_config_entry *entry, void *payload)
{
	int *count = payload;
	GIT_UNUSED(entry);
	(*count)++;
	return 0;
}


// Source: read.c
// Lines 382-388
