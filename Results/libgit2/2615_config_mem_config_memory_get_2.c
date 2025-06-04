static int config_memory_get(git_config_backend *backend, const char *key, git_config_entry **out)
{
	config_memory_backend *memory_backend = (config_memory_backend *) backend;
	return git_config_entries_get(out, memory_backend->entries, key);
}


// Source: config_mem.c
// Lines 101-105
