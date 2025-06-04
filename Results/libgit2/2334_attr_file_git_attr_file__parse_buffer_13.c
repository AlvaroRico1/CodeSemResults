int git_attr_file__parse_buffer(
	git_repository *repo, git_attr_file *attrs, const char *data, bool allow_macros)
{
	const char *scan = data, *context = NULL;
	git_attr_rule *rule = NULL;
	int error = 0;

	/* If subdir file path, convert context for file paths */
	if (attrs->entry && git_fs_path_root(attrs->entry->path) < 0 &&
	    !git__suffixcmp(attrs->entry->path, "/" GIT_ATTR_FILE))
		context = attrs->entry->path;

	if (git_mutex_lock(&attrs->lock) < 0) {
		git_error_set(GIT_ERROR_OS, "failed to lock attribute file");
		return -1;
	}

	while (!error && *scan) {
		/* Allocate rule if needed, otherwise re-use previous rule */
		if (!rule) {
			rule = git__calloc(1, sizeof(*rule));
			GIT_ERROR_CHECK_ALLOC(rule);
		} else
			git_attr_rule__clear(rule);

		rule->match.flags = GIT_ATTR_FNMATCH_ALLOWNEG | GIT_ATTR_FNMATCH_ALLOWMACRO;

		/* Parse the next "pattern attr attr attr" line */
		if ((error = git_attr_fnmatch__parse(&rule->match, &attrs->pool, context, &scan)) < 0 ||
		    (error = git_attr_assignment__parse(repo, &attrs->pool, &rule->assigns, &scan)) < 0)
		{
			if (error != GIT_ENOTFOUND)
				goto out;
			error = 0;
			continue;
		}

		if (rule->match.flags & GIT_ATTR_FNMATCH_MACRO) {
			/* TODO: warning if macro found in file below repo root */
			if (!allow_macros)
				continue;
			if ((error = git_attr_cache__insert_macro(repo, rule)) < 0)
				goto out;
		} else if ((error = git_vector_insert(&attrs->rules, rule)) < 0)
			goto out;

		rule = NULL;
	}

out:
	git_mutex_unlock(&attrs->lock);
	git_attr_rule__free(rule);

	return error;
}


// Source: attr_file.c
// Lines 340-394
