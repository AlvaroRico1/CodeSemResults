static int cb(const git_config_entry *entry, void *data)
{
	int *n = (int *) data;

	GIT_UNUSED(entry);

	(*n)++;

	return 0;
}


// Source: multivar.c
// Lines 38-47
