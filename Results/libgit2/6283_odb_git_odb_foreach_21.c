int git_odb_foreach(git_odb *db, git_odb_foreach_cb cb, void *payload)
{
	unsigned int i;
	git_vector backends = GIT_VECTOR_INIT;
	backend_internal *internal;
	int error = 0;

	/* Make a copy of the backends vector to invoke the callback without holding the lock. */
	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		goto cleanup;
	}
	error = git_vector_dup(&backends, &db->backends, NULL);
	git_mutex_unlock(&db->lock);

	if (error < 0)
		goto cleanup;

	git_vector_foreach(&backends, i, internal) {
		git_odb_backend *b = internal->backend;
		error = b->foreach(b, cb, payload);
		if (error != 0)
			goto cleanup;
	}


// Source: odb.c
// Lines 1425-1448
