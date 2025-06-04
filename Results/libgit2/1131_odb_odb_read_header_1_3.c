static int odb_read_header_1(
	size_t *len_p, git_object_t *type_p, git_odb *db,
	const git_oid *id, bool only_refreshed)
{
	size_t i;
	git_object_t ht;
	bool passthrough = false;
	int error;

	if (!only_refreshed && (ht = odb_hardcoded_type(id)) != GIT_OBJECT_INVALID) {
		*type_p = ht;
		*len_p = 0;
		return 0;
	}

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	for (i = 0; i < db->backends.length; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		if (only_refreshed && !b->refresh)
			continue;

		if (!b->read_header) {
			passthrough = true;
			continue;
		}

		error = b->read_header(len_p, type_p, b, id);

		switch (error) {
		case GIT_PASSTHROUGH:
			passthrough = true;
			break;
		case GIT_ENOTFOUND:
			break;
		default:
			git_mutex_unlock(&db->lock);
			return error;
		}
	}


// Source: odb.c
// Lines 1080-1123
