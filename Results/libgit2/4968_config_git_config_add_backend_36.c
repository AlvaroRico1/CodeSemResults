int git_config_add_backend(
	git_config *cfg,
	git_config_backend *backend,
	git_config_level_t level,
	const git_repository *repo,
	int force)
{
	backend_internal *internal;
	int result;

	GIT_ASSERT_ARG(cfg);
	GIT_ASSERT_ARG(backend);

	GIT_ERROR_CHECK_VERSION(backend, GIT_CONFIG_BACKEND_VERSION, "git_config_backend");

	if ((result = backend->open(backend, level, repo)) < 0)
		return result;

	internal = git__malloc(sizeof(backend_internal));
	GIT_ERROR_CHECK_ALLOC(internal);

	memset(internal, 0x0, sizeof(backend_internal));

	internal->backend = backend;
	internal->level = level;

	if ((result = git_config__add_internal(cfg, internal, level, force)) < 0) {
		git__free(internal);
		return result;
	}

	return 0;
}


// Source: config.c
// Lines 310-342
