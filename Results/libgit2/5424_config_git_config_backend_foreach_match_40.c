int git_config_backend_foreach_match(
	git_config_backend *backend,
	const char *regexp,
	git_config_foreach_cb cb,
	void *payload)
{
	git_config_entry *entry;
	git_config_iterator *iter;
	git_regexp regex;
	int error = 0;

	GIT_ASSERT_ARG(backend);
	GIT_ASSERT_ARG(cb);

	if (regexp && git_regexp_compile(&regex, regexp, 0) < 0)
		return -1;

	if ((error = backend->iterator(&iter, backend)) < 0) {
		iter = NULL;
		return -1;
	}

	while (!(iter->next(&entry, iter) < 0)) {
		/* skip non-matching keys if regexp was provided */
		if (regexp && git_regexp_match(&regex, entry->name) != 0)
			continue;

		/* abort iterator on non-zero return value */
		if ((error = cb(entry, payload)) != 0) {
			git_error_set_after_callback(error);
			break;
		}
	}

	if (regexp != NULL)
		git_regexp_dispose(&regex);

	iter->free(iter);

	return error;
}


// Source: config.c
// Lines 508-548
