int git_config_foreach_match(
	const git_config *cfg,
	const char *regexp,
	git_config_foreach_cb cb,
	void *payload)
{
	int error;
	git_config_iterator *iter;
	git_config_entry *entry;

	if ((error = git_config_iterator_glob_new(&iter, cfg, regexp)) < 0)
		return error;

	while (!(error = git_config_next(&entry, iter))) {
		if ((error = cb(entry, payload)) != 0) {
			git_error_set_after_callback(error);
			break;
		}
	}

	git_config_iterator_free(iter);

	if (error == GIT_ITEROVER)
		error = 0;

	return error;
}


// Source: config.c
// Lines 550-576
