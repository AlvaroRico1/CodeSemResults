static int count_cfg_entries_and_compare_levels(
	const git_config_entry *entry, void *payload)
{
	int *count = payload;

	if (!strcmp(entry->value, "7") || !strcmp(entry->value, "17"))
		cl_assert(entry->level == GIT_CONFIG_LEVEL_GLOBAL);
	else
		cl_assert(entry->level == GIT_CONFIG_LEVEL_SYSTEM);

	(*count)++;
	return 0;
}


// Source: read.c
// Lines 295-307
