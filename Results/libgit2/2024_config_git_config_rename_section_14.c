int git_config_rename_section(
	git_repository *repo,
	const char *old_section_name,
	const char *new_section_name)
{
	git_config *config;
	git_str pattern = GIT_STR_INIT, replace = GIT_STR_INIT;
	int error = 0;
	struct rename_data data;

	git_str_puts_escape_regex(&pattern, old_section_name);

	if ((error = git_str_puts(&pattern, "\\..+")) < 0)
		goto cleanup;

	if ((error = git_repository_config__weakptr(&config, repo)) < 0)
		goto cleanup;

	data.config  = config;
	data.name    = &replace;
	data.old_len = strlen(old_section_name) + 1;

	if ((error = git_str_join(&replace, '.', new_section_name, "")) < 0)
		goto cleanup;

	if (new_section_name != NULL &&
	    (error = normalize_section(replace.ptr, strchr(replace.ptr, '.'))) < 0)
	{
		git_error_set(
			GIT_ERROR_CONFIG, "invalid config section '%s'", new_section_name);
		goto cleanup;
	}

	error = git_config_foreach_match(
		config, git_str_cstr(&pattern), rename_config_entries_cb, &data);

cleanup:
	git_str_dispose(&pattern);
	git_str_dispose(&replace);

	return error;
}


// Source: config.c
// Lines 1518-1559
