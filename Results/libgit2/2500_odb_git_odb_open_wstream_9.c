int git_odb_open_wstream(
	git_odb_stream **stream, git_odb *db, git_object_size_t size, git_object_t type)
{
	size_t i, writes = 0;
	int error = GIT_ERROR;
	git_hash_ctx *ctx = NULL;

	GIT_ASSERT_ARG(stream);
	GIT_ASSERT_ARG(db);

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	error = GIT_ERROR;
	for (i = 0; i < db->backends.length && error < 0; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		/* we don't write in alternates! */
		if (internal->is_alternate)
			continue;

		if (b->writestream != NULL) {
			++writes;
			error = b->writestream(stream, b, size, type);
		} else if (b->write != NULL) {
			++writes;
			error = init_fake_wstream(stream, b, size, type);
		}
	}


// Source: odb.c
// Lines 1522-1552
