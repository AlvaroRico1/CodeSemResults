static int multivar_iter_next(git_config_entry **entry, git_config_iterator *_iter)
{
	multivar_iter *iter = (multivar_iter *) _iter;
	int error = 0;

	while ((error = iter->iter->next(entry, iter->iter)) == 0) {
		if (git__strcmp(iter->name, (*entry)->name))
			continue;

		if (!iter->have_regex)
			return 0;

		if (git_regexp_match(&iter->regex, (*entry)->value) == 0)
			return 0;
	}

	return error;
}


// Source: config.c
// Lines 1027-1044
