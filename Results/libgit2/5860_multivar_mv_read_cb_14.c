static int mv_read_cb(const git_config_entry *entry, void *data)
{
	int *n = (int *) data;

	if (!strcmp(entry->name, _name))
		(*n)++;

	return 0;
}


// Source: multivar.c
// Lines 15-23
