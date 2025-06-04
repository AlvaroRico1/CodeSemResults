static int all_iter_glob_next(git_config_entry **entry, git_config_iterator *_iter)
{
	int error;
	all_iter *iter = (all_iter *) _iter;

	/*
	 * We use the "normal" function to grab the next one across
	 * backends and then apply the regex
	 */
	while ((error = all_iter_next(entry, _iter)) == 0) {
		/* skip non-matching keys if regexp was provided */
		if (git_regexp_match(&iter->regex, (*entry)->name) != 0)
			continue;

		/* and simply return if we like the entry's name */
		return 0;
	}

	return error;
}


// Source: config.c
// Lines 419-438
