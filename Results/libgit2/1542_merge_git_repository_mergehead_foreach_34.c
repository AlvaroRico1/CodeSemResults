int git_repository_mergehead_foreach(
	git_repository *repo,
	git_repository_mergehead_foreach_cb cb,
	void *payload)
{
	git_str merge_head_path = GIT_STR_INIT, merge_head_file = GIT_STR_INIT;
	char *buffer, *line;
	size_t line_num = 1;
	git_oid oid;
	int error = 0;

	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(cb);

	if ((error = git_str_joinpath(&merge_head_path, repo->gitdir,
		GIT_MERGE_HEAD_FILE)) < 0)
		return error;

	if ((error = git_futils_readbuffer(&merge_head_file,
		git_str_cstr(&merge_head_path))) < 0)
		goto cleanup;

	buffer = merge_head_file.ptr;

	while ((line = git__strsep(&buffer, "\n")) != NULL) {
		if (strlen(line) != GIT_OID_HEXSZ) {
			git_error_set(GIT_ERROR_INVALID, "unable to parse OID - invalid length");
			error = -1;
			goto cleanup;
		}

		if ((error = git_oid_fromstr(&oid, line)) < 0)
			goto cleanup;

		if ((error = cb(&oid, payload)) != 0) {
			git_error_set_after_callback(error);
			goto cleanup;
		}

		++line_num;
	}

	if (*buffer) {
		git_error_set(GIT_ERROR_MERGE, "no EOL at line %"PRIuZ, line_num);
		error = -1;
		goto cleanup;
	}

cleanup:
	git_str_dispose(&merge_head_path);
	git_str_dispose(&merge_head_file);

	return error;
}


// Source: merge.c
// Lines 589-642
