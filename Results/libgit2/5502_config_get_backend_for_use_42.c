static int get_backend_for_use(git_config_backend **out,
	git_config *cfg, const char *name, backend_use use)
{
	size_t i;
	backend_internal *backend;

	*out = NULL;

	if (git_vector_length(&cfg->backends) == 0) {
		git_error_set(GIT_ERROR_CONFIG,
			"cannot %s value for '%s' when no config backends exist",
			uses[use], name);
		return GIT_ENOTFOUND;
	}

	git_vector_foreach(&cfg->backends, i, backend) {
		if (!backend->backend->readonly) {
			*out = backend->backend;
			return 0;
		}
	}

	git_error_set(GIT_ERROR_CONFIG,
		"cannot %s value for '%s' when all config backends are readonly",
		uses[use], name);
	return GIT_ENOTFOUND;
}


// Source: config.c
// Lines 592-618
