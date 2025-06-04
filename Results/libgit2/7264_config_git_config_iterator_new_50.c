int git_config_iterator_new(git_config_iterator **out, const git_config *cfg)
{
	all_iter *iter;

	iter = git__calloc(1, sizeof(all_iter));
	GIT_ERROR_CHECK_ALLOC(iter);

	iter->parent.free = all_iter_free;
	iter->parent.next = all_iter_next;

	iter->i = cfg->backends.length;
	iter->cfg = cfg;

	*out = (git_config_iterator *) iter;

	return 0;
}


// Source: config.c
// Lines 458-474
