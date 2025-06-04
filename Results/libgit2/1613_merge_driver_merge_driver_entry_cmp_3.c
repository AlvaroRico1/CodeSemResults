static int merge_driver_entry_cmp(const void *a, const void *b)
{
	const git_merge_driver_entry *entry_a = a;
	const git_merge_driver_entry *entry_b = b;

	return strcmp(entry_a->name, entry_b->name);
}


// Source: merge_driver.c
// Lines 139-145
