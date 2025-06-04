static void all_iter_glob_free(git_config_iterator *_iter)
{
	all_iter *iter = (all_iter *) _iter;

	git_regexp_dispose(&iter->regex);
	all_iter_free(_iter);
}


// Source: config.c
// Lines 450-456
