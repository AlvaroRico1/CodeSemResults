int git_config_new(git_config **out)
{
	git_config *cfg;

	cfg = git__malloc(sizeof(git_config));
	GIT_ERROR_CHECK_ALLOC(cfg);

	memset(cfg, 0x0, sizeof(git_config));

	if (git_vector_init(&cfg->backends, 3, config_backend_cmp) < 0) {
		git__free(cfg);
		return -1;
	}

	*out = cfg;
	GIT_REFCOUNT_INC(cfg);
	return 0;
}


// Source: config.c
// Lines 81-98
