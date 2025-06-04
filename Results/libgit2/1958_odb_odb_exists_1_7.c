static int odb_exists_1(
	git_odb *db,
	const git_oid *id,
	bool only_refreshed)
{
	size_t i;
	bool found = false;
	int error;

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	for (i = 0; i < db->backends.length && !found; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		if (only_refreshed && !b->refresh)
			continue;

		if (b->exists != NULL)
			found = (bool)b->exists(b, id);
	}


// Source: odb.c
// Lines 786-808
