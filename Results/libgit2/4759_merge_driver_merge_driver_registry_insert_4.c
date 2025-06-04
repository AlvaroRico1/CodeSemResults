static int merge_driver_registry_insert(
	const char *name, git_merge_driver *driver)
{
	git_merge_driver_entry *entry;

	entry = git__calloc(1, sizeof(git_merge_driver_entry) + strlen(name) + 1);
	GIT_ERROR_CHECK_ALLOC(entry);

	strcpy(entry->name, name);
	entry->driver = driver;

	return git_vector_insert_sorted(
		&merge_driver_registry.drivers, entry, NULL);
}


// Source: merge_driver.c
// Lines 183-196
