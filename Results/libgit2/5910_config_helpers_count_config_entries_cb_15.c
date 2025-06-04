static int count_config_entries_cb(
	const git_config_entry *entry,
	void *payload)
{
	int *how_many = (int *)payload;

	GIT_UNUSED(entry);

	(*how_many)++;

	return 0;
}


// Source: config_helpers.c
// Lines 41-52
