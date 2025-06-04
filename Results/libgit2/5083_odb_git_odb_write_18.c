int git_odb_write(
	git_oid *oid, git_odb *db, const void *data, size_t len, git_object_t type)
{
	size_t i;
	int error;
	git_odb_stream *stream;

	GIT_ASSERT_ARG(oid);
	GIT_ASSERT_ARG(db);

	if ((error = git_odb_hash(oid, data, len, type)) < 0)
		return error;

	if (git_oid_is_zero(oid))
		return error_null_oid(GIT_EINVALID, "cannot write object");

	if (git_odb__freshen(db, oid))
		return 0;

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	for (i = 0, error = GIT_ERROR; i < db->backends.length && error < 0; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		/* we don't write in alternates! */
		if (internal->is_alternate)
			continue;

		if (b->write != NULL)
			error = b->write(b, oid, data, len, type);
	}


// Source: odb.c
// Lines 1456-1489
