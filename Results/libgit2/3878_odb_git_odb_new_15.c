int git_odb_new(git_odb **out)
{
	git_odb *db = git__calloc(1, sizeof(*db));
	GIT_ERROR_CHECK_ALLOC(db);

	if (git_mutex_init(&db->lock) < 0) {
		git__free(db);
		return -1;
	}
	if (git_cache_init(&db->own_cache) < 0) {
		git_mutex_free(&db->lock);
		git__free(db);
		return -1;
	}
	if (git_vector_init(&db->backends, 4, backend_sort_cmp) < 0) {
		git_cache_dispose(&db->own_cache);
		git_mutex_free(&db->lock);
		git__free(db);
		return -1;
	}

	*out = db;
	GIT_REFCOUNT_INC(db);
	return 0;
}


// Source: odb.c
// Lines 445-469
