static void config_memory_free(git_config_backend *_backend)
{
	config_memory_backend *backend = (config_memory_backend *)_backend;

	if (backend == NULL)
		return;

	git_config_entries_free(backend->entries);
	git_str_dispose(&backend->cfg);
	git__free(backend);
}


// Source: config_mem.c
// Lines 173-183
