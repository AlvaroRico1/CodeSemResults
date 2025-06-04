int git_config_iterator_glob_new(git_config_iterator **out, const git_config *cfg, const char *regexp)
{
	all_iter *iter;
	int result;

	if (regexp == NULL)
		return git_config_iterator_new(out, cfg);

	iter = git__calloc(1, sizeof(all_iter));
	GIT_ERROR_CHECK_ALLOC(iter);

	if ((result = git_regexp_compile(&iter->regex, regexp, 0)) < 0) {
		git__free(iter);
		return -1;
	}

	iter->parent.next = all_iter_glob_next;
	iter->parent.free = all_iter_glob_free;
	iter->i = cfg->backends.length;
	iter->cfg = cfg;

	*out = (git_config_iterator *) iter;

	return 0;
}


// Source: config.c
// Lines 476-500
