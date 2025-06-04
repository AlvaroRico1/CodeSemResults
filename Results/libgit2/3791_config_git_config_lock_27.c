int git_config_lock(git_transaction **out, git_config *cfg)
{
	int error;
	git_config_backend *backend;
	backend_internal *internal;

	GIT_ASSERT_ARG(cfg);

	internal = git_vector_get(&cfg->backends, 0);
	if (!internal || !internal->backend) {
		git_error_set(GIT_ERROR_CONFIG, "cannot lock; the config has no backends");
		return -1;
	}
	backend = internal->backend;

	if ((error = backend->lock(backend)) < 0)
		return error;

	return git_transaction_config_new(out, cfg);
}


// Source: config.c
// Lines 1247-1266
