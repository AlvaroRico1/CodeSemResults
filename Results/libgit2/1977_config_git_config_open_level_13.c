int git_config_open_level(
	git_config **cfg_out,
	const git_config *cfg_parent,
	git_config_level_t level)
{
	git_config *cfg;
	backend_internal *internal;
	int res;

	if ((res = find_backend_by_level(&internal, cfg_parent, level)) < 0)
		return res;

	if ((res = git_config_new(&cfg)) < 0)
		return res;

	if ((res = git_config__add_internal(cfg, internal, level, true)) < 0) {
		git_config_free(cfg);
		return res;
	}

	*cfg_out = cfg;

	return 0;
}


// Source: config.c
// Lines 285-308
