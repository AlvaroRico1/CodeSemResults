int git_refdb_backend_fs(
	git_refdb_backend **backend_out,
	git_repository *repository)
{
	int t = 0;
	git_str gitpath = GIT_STR_INIT;
	refdb_fs_backend *backend;

	backend = git__calloc(1, sizeof(refdb_fs_backend));
	GIT_ERROR_CHECK_ALLOC(backend);
	if (git_mutex_init(&backend->prlock) < 0) {
		git__free(backend);
		return -1;
	}


	if (git_refdb_init_backend(&backend->parent, GIT_REFDB_BACKEND_VERSION) < 0)
		goto fail;

	backend->repo = repository;

	if (repository->gitdir) {
		backend->gitpath = setup_namespace(repository, repository->gitdir);

		if (backend->gitpath == NULL)
			goto fail;
	}

	if (repository->commondir) {
		backend->commonpath = setup_namespace(repository, repository->commondir);

		if (backend->commonpath == NULL)
			goto fail;
	}

	if (git_str_joinpath(&gitpath, backend->commonpath, GIT_PACKEDREFS_FILE) < 0 ||
		git_sortedcache_new(
			&backend->refcache, offsetof(struct packref, name),
			NULL, NULL, packref_cmp, git_str_cstr(&gitpath)) < 0)
		goto fail;

	git_str_dispose(&gitpath);

	if (!git_repository__configmap_lookup(&t, backend->repo, GIT_CONFIGMAP_IGNORECASE) && t) {
		backend->iterator_flags |= GIT_ITERATOR_IGNORE_CASE;
		backend->direach_flags  |= GIT_FS_PATH_DIR_IGNORE_CASE;
	}
	if (!git_repository__configmap_lookup(&t, backend->repo, GIT_CONFIGMAP_PRECOMPOSE) && t) {
		backend->iterator_flags |= GIT_ITERATOR_PRECOMPOSE_UNICODE;
		backend->direach_flags  |= GIT_FS_PATH_DIR_PRECOMPOSE_UNICODE;
	}
	if ((!git_repository__configmap_lookup(&t, backend->repo, GIT_CONFIGMAP_FSYNCOBJECTFILES) && t) ||
		git_repository__fsync_gitdir)
		backend->fsync = 1;
	backend->iterator_flags |= GIT_ITERATOR_DESCEND_SYMLINKS;

	backend->parent.exists = &refdb_fs_backend__exists;
	backend->parent.lookup = &refdb_fs_backend__lookup;
	backend->parent.iterator = &refdb_fs_backend__iterator;
	backend->parent.write = &refdb_fs_backend__write;
	backend->parent.del = &refdb_fs_backend__delete;
	backend->parent.rename = &refdb_fs_backend__rename;
	backend->parent.compress = &refdb_fs_backend__compress;
	backend->parent.lock = &refdb_fs_backend__lock;
	backend->parent.unlock = &refdb_fs_backend__unlock;
	backend->parent.has_log = &refdb_reflog_fs__has_log;
	backend->parent.ensure_log = &refdb_reflog_fs__ensure_log;
	backend->parent.free = &refdb_fs_backend__free;
	backend->parent.reflog_read = &refdb_reflog_fs__read;
	backend->parent.reflog_write = &refdb_reflog_fs__write;
	backend->parent.reflog_rename = &refdb_reflog_fs__rename;
	backend->parent.reflog_delete = &refdb_reflog_fs__delete;

	*backend_out = (git_refdb_backend *)backend;
	return 0;

fail:
	git_mutex_free(&backend->prlock);
	git_str_dispose(&gitpath);
	git__free(backend->gitpath);
	git__free(backend->commonpath);
	git__free(backend);
	return -1;
}


// Source: refdb_fs.c
// Lines 2385-2468
