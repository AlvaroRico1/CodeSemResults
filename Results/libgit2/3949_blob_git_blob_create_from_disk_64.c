int git_blob_create_from_disk(
	git_oid *id, git_repository *repo, const char *path)
{
	int error;
	git_str full_path = GIT_STR_INIT;
	const char *workdir, *hintpath = NULL;

	if ((error = git_fs_path_prettify(&full_path, path, NULL)) < 0) {
		git_str_dispose(&full_path);
		return error;
	}

	workdir  = git_repository_workdir(repo);

	if (workdir && !git__prefixcmp(full_path.ptr, workdir))
		hintpath = full_path.ptr + strlen(workdir);

	error = git_blob__create_from_paths(
		id, NULL, repo, git_str_cstr(&full_path), hintpath, 0, !!hintpath);

	git_str_dispose(&full_path);
	return error;
}


// Source: blob.c
// Lines 276-298
