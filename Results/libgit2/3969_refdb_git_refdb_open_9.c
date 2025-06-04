int git_refdb_open(git_refdb **out, git_repository *repo)
{
	git_refdb *db;
	git_refdb_backend *dir;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);

	*out = NULL;

	if (git_refdb_new(&db, repo) < 0)
		return -1;

	/* Add the default (filesystem) backend */
	if (git_refdb_backend_fs(&dir, repo) < 0) {
		git_refdb_free(db);
		return -1;
	}

	db->repo = repo;
	db->backend = dir;

	*out = db;
	return 0;
}


// Source: refdb.c
// Lines 40-64
