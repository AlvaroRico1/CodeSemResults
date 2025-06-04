static void all_iter_free(git_config_iterator *_iter)
{
	all_iter *iter = (all_iter *) _iter;

	if (iter->current)
		iter->current->free(iter->current);

	git__free(iter);
}


// Source: config.c
// Lines 440-448
