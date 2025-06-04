static int get_entry(
	git_config_entry **out,
	const git_config *cfg,
	const char *name,
	bool normalize_name,
	int want_errors)
{
	int res = GIT_ENOTFOUND;
	const char *key = name;
	char *normalized = NULL;
	size_t i;
	backend_internal *internal;

	*out = NULL;

	if (normalize_name) {
		if ((res = git_config__normalize_name(name, &normalized)) < 0)
			goto cleanup;
		key = normalized;
	}

	res = GIT_ENOTFOUND;
	git_vector_foreach(&cfg->backends, i, internal) {
		if (!internal || !internal->backend)
			continue;

		res = internal->backend->get(internal->backend, key, out);
		if (res != GIT_ENOTFOUND)
			break;
	}

	git__free(normalized);

cleanup:
	if (res == GIT_ENOTFOUND)
		res = (want_errors > GET_ALL_ERRORS) ? 0 : config_error_notfound(name);
	else if (res && (want_errors == GET_NO_ERRORS)) {
		git_error_clear();
		res = 0;
	}

	return res;
}


// Source: config.c
// Lines 715-757
