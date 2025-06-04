int git_config_open_ondisk(git_config **out, const char *path)
{
	int error;
	git_config *config;

	*out = NULL;

	if (git_config_new(&config) < 0)
		return -1;

	if ((error = git_config_add_file_ondisk(config, path, GIT_CONFIG_LEVEL_LOCAL, NULL, 0)) < 0)
		git_config_free(config);
	else
		*out = config;

	return error;
}


// Source: config.c
// Lines 135-151
