static void odb_free(git_odb *db)
{
	size_t i;
	bool locked = true;

	if (git_mutex_lock(&db->lock) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		locked = false;
	}
	for (i = 0; i < db->backends.length; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *backend = internal->backend;

		backend->free(backend);

		git__free(internal);
	}


// Source: odb.c
// Lines 749-765
