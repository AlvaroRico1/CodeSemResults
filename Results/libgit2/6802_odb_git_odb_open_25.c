int git_odb_open(git_odb **out, const char *objects_dir)
{
	git_odb *db;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(objects_dir);

	*out = NULL;

	if (git_odb_new(&db) < 0)
		return -1;

	if (git_odb__add_default_backends(db, objects_dir, 0, 0) < 0) {
		git_odb_free(db);
		return -1;
	}

	*out = db;
	return 0;
}


// Source: odb.c
// Lines 710-729
