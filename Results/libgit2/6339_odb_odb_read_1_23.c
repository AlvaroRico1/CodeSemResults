static int odb_read_1(git_odb_object **out, git_odb *db, const git_oid *id,
		bool only_refreshed)
{
	size_t i;
	git_rawobj raw;
	git_odb_object *object;
	git_oid hashed;
	bool found = false;
	int error = 0;

	if (!only_refreshed) {
		if ((error = odb_read_hardcoded(&found, &raw, id)) < 0)
			return error;
	}

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	for (i = 0; i < db->backends.length && !found; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		if (only_refreshed && !b->refresh)
			continue;

		if (b->read != NULL) {
			error = b->read(&raw.data, &raw.len, &raw.type, b, id);
			if (error == GIT_PASSTHROUGH || error == GIT_ENOTFOUND)
				continue;

			if (error < 0) {
				git_mutex_unlock(&db->lock);
				return error;
			}

			found = true;
		}
	}


// Source: odb.c
// Lines 1182-1220
