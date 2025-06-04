static int add_backend_internal(
	git_odb *odb, git_odb_backend *backend,
	int priority, bool is_alternate, ino_t disk_inode)
{
	backend_internal *internal;

	GIT_ASSERT_ARG(odb);
	GIT_ASSERT_ARG(backend);

	GIT_ERROR_CHECK_VERSION(backend, GIT_ODB_BACKEND_VERSION, "git_odb_backend");

	/* Check if the backend is already owned by another ODB */
	GIT_ASSERT(!backend->odb || backend->odb == odb);

	internal = git__malloc(sizeof(backend_internal));
	GIT_ERROR_CHECK_ALLOC(internal);

	internal->backend = backend;
	internal->priority = priority;
	internal->is_alternate = is_alternate;
	internal->disk_inode = disk_inode;

	if (git_mutex_lock(&odb->lock) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return -1;
	}
	if (git_vector_insert(&odb->backends, internal) < 0) {
		git_mutex_unlock(&odb->lock);
		git__free(internal);
		return -1;
	}
	git_vector_sort(&odb->backends);
	internal->backend->odb = odb;
	git_mutex_unlock(&odb->lock);
	return 0;
}


// Source: odb.c
// Lines 471-506
