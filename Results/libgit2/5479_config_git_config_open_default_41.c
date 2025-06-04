int git_config_open_default(git_config **out)
{
	int error;
	git_config *cfg = NULL;
	git_str buf = GIT_STR_INIT;

	if ((error = git_config_new(&cfg)) < 0)
		return error;

	if (!git_config__find_global(&buf) ||
	    !git_config__global_location(&buf)) {
		error = git_config_add_file_ondisk(cfg, buf.ptr,
			GIT_CONFIG_LEVEL_GLOBAL, NULL, 0);
	}

	if (!error && !git_config__find_xdg(&buf))
		error = git_config_add_file_ondisk(cfg, buf.ptr,
			GIT_CONFIG_LEVEL_XDG, NULL, 0);

	if (!error && !git_config__find_system(&buf))
		error = git_config_add_file_ondisk(cfg, buf.ptr,
			GIT_CONFIG_LEVEL_SYSTEM, NULL, 0);

	if (!error && !git_config__find_programdata(&buf))
		error = git_config_add_file_ondisk(cfg, buf.ptr,
			GIT_CONFIG_LEVEL_PROGRAMDATA, NULL, 0);

	git_str_dispose(&buf);

	if (error) {
		git_config_free(cfg);
		cfg = NULL;
	}

	*out = cfg;

	return error;
}


// Source: config.c
// Lines 1208-1245
