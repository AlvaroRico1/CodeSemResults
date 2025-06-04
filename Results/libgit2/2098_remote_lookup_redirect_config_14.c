static int lookup_redirect_config(
	git_remote_redirect_t *out,
	git_repository *repo)
{
	git_config *config;
	const char *value;
	int bool_value, error = 0;

	if (!repo) {
		*out = GIT_REMOTE_REDIRECT_INITIAL;
		return 0;
	}

	if ((error = git_repository_config_snapshot(&config, repo)) < 0)
		goto done;

	if ((error = git_config_get_string(&value, config, "http.followRedirects")) < 0) {
		if (error == GIT_ENOTFOUND) {
			*out = GIT_REMOTE_REDIRECT_INITIAL;
			error = 0;
		}

		goto done;
	}

	if (git_config_parse_bool(&bool_value, value) == 0) {
		*out = bool_value ? GIT_REMOTE_REDIRECT_ALL :
		                    GIT_REMOTE_REDIRECT_NONE;
	} else if (strcasecmp(value, "initial") == 0) {
		*out = GIT_REMOTE_REDIRECT_INITIAL;
	} else {
		git_error_set(GIT_ERROR_CONFIG, "invalid configuration setting '%s' for 'http.followRedirects'", value);
		error = -1;
	}

done:
	git_config_free(config);
	return error;
}


// Source: remote.c
// Lines 861-899
