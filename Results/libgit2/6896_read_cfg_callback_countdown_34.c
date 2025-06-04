static int cfg_callback_countdown(const git_config_entry *entry, void *payload)
{
	int *count = payload;
	GIT_UNUSED(entry);
	(*count)--;
	if (*count == 0)
		return -100;
	return 0;
}


// Source: read.c
// Lines 309-317
