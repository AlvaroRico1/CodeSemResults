static int rename_remote_references(
	git_repository *repo,
	const char *old_name,
	const char *new_name)
{
	int error;
	git_str buf = GIT_STR_INIT;
	git_reference *ref;
	git_reference_iterator *iter;

	if ((error = git_str_printf(&buf, GIT_REFS_REMOTES_DIR "%s/*", old_name)) < 0)
		return error;

	error = git_reference_iterator_glob_new(&iter, repo, git_str_cstr(&buf));
	git_str_dispose(&buf);

	if (error < 0)
		return error;

	while ((error = git_reference_next(&ref, iter)) == 0) {
		if ((error = rename_one_remote_reference(ref, old_name, new_name)) < 0)
			break;
	}

	git_reference_iterator_free(iter);

	return (error == GIT_ITEROVER) ? 0 : error;
}


// Source: remote.c
// Lines 2399-2426
