static int config_memory_iterator(
	git_config_iterator **iter,
	git_config_backend *backend)
{
	config_memory_backend *memory_backend = (config_memory_backend *) backend;
	git_config_entries *entries;
	int error;

	if ((error = git_config_entries_dup(&entries, memory_backend->entries)) < 0)
		goto out;

	if ((error = git_config_entries_iterator_new(iter, entries)) < 0)
		goto out;

out:
	/* Let iterator delete duplicated entries when it's done */
	git_config_entries_free(entries);
	return error;
}


// Source: config_mem.c
// Lines 107-125
