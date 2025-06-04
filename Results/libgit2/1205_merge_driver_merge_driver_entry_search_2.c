static int merge_driver_entry_search(const void *a, const void *b)
{
	const char *name_a = a;
	const git_merge_driver_entry *entry_b = b;

	return strcmp(name_a, entry_b->name);
}


// Source: merge_driver.c
// Lines 147-153
