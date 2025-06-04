static int make_origin(git_blame__origin **out, git_commit *commit, const char *path)
{
	git_blame__origin *o;
	git_object *blob;
	size_t path_len = strlen(path), alloc_len;
	int error = 0;

	if ((error = git_object_lookup_bypath(&blob, (git_object*)commit,
			path, GIT_OBJECT_BLOB)) < 0)
		return error;

	GIT_ERROR_CHECK_ALLOC_ADD(&alloc_len, sizeof(*o), path_len);
	GIT_ERROR_CHECK_ALLOC_ADD(&alloc_len, alloc_len, 1);
	o = git__calloc(1, alloc_len);
	GIT_ERROR_CHECK_ALLOC(o);

	o->commit = commit;
	o->blob = (git_blob *) blob;
	o->refcnt = 1;
	strcpy(o->path, path);

	*out = o;

	return 0;
}


// Source: blame_git.c
// Lines 38-62
