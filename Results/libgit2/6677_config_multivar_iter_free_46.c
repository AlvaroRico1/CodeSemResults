static void multivar_iter_free(git_config_iterator *_iter)
{
	multivar_iter *iter = (multivar_iter *) _iter;

	iter->iter->free(iter->iter);

	git__free(iter->name);
	if (iter->have_regex)
		git_regexp_dispose(&iter->regex);
	git__free(iter);
}


// Source: config.c
// Lines 1046-1056
