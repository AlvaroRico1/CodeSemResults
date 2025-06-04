static int multivar_cb(const git_config_entry *entry, void *data)
{
	int *n = (int *)data;

	cl_assert_equal_s(entry->value, "newurl");

	(*n)++;

	return 0;
}


// Source: write.c
// Lines 236-245
