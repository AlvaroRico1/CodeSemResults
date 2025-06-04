static int all_iter_next(git_config_entry **entry, git_config_iterator *_iter)
{
	all_iter *iter = (all_iter *) _iter;
	backend_internal *internal;
	git_config_backend *backend;
	size_t i;
	int error = 0;

	if (iter->current != NULL &&
	    (error = iter->current->next(entry, iter->current)) == 0) {
		return 0;
	}

	if (error < 0 && error != GIT_ITEROVER)
		return error;

	do {
		if (find_next_backend(&i, iter->cfg, iter->i) < 0)
			return GIT_ITEROVER;

		internal = git_vector_get(&iter->cfg->backends, i - 1);
		backend = internal->backend;
		iter->i = i - 1;

		if (iter->current)
			iter->current->free(iter->current);

		iter->current = NULL;
		error = backend->iterator(&iter->current, backend);
		if (error == GIT_ENOTFOUND)
			continue;

		if (error < 0)
			return error;

		error = iter->current->next(entry, iter->current);
		/* If this backend is empty, then keep going */
		if (error == GIT_ITEROVER)
			continue;

		return error;

	} while(1);

	return GIT_ITEROVER;
}


// Source: config.c
// Lines 372-417
