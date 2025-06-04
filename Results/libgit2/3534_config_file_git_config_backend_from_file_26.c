int git_config_backend_from_file(git_config_backend **out, const char *path)
{
	config_file_backend *backend;

	backend = git__calloc(1, sizeof(config_file_backend));
	GIT_ERROR_CHECK_ALLOC(backend);

	backend->parent.version = GIT_CONFIG_BACKEND_VERSION;
	git_mutex_init(&backend->values_mutex);

	backend->file.path = git__strdup(path);
	GIT_ERROR_CHECK_ALLOC(backend->file.path);
	git_array_init(backend->file.includes);

	backend->parent.open = config_file_open;
	backend->parent.get = config_file_get;
	backend->parent.set = config_file_set;
	backend->parent.set_multivar = config_file_set_multivar;
	backend->parent.del = config_file_delete;
	backend->parent.del_multivar = config_file_delete_multivar;
	backend->parent.iterator = config_file_iterator;
	backend->parent.snapshot = config_file_snapshot;
	backend->parent.lock = config_file_lock;
	backend->parent.unlock = config_file_unlock;
	backend->parent.free = config_file_free;

	*out = (git_config_backend *)backend;

	return 0;
}


// Source: config_file.c
// Lines 496-525
