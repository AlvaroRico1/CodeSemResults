static int load_alternates(git_odb *odb, const char *objects_dir, int alternate_depth)
{
	git_str alternates_path = GIT_STR_INIT;
	git_str alternates_buf = GIT_STR_INIT;
	char *buffer;
	const char *alternate;
	int result = 0;

	/* Git reports an error, we just ignore anything deeper */
	if (alternate_depth > GIT_ALTERNATES_MAX_DEPTH)
		return 0;

	if (git_str_joinpath(&alternates_path, objects_dir, GIT_ALTERNATES_FILE) < 0)
		return -1;

	if (git_fs_path_exists(alternates_path.ptr) == false) {
		git_str_dispose(&alternates_path);
		return 0;
	}

	if (git_futils_readbuffer(&alternates_buf, alternates_path.ptr) < 0) {
		git_str_dispose(&alternates_path);
		return -1;
	}

	buffer = (char *)alternates_buf.ptr;

	/* add each alternate as a new backend; one alternate per line */
	while ((alternate = git__strtok(&buffer, "\r\n")) != NULL) {
		if (*alternate == '\0' || *alternate == '#')
			continue;

		/*
		 * Relative path: build based on the current `objects`
		 * folder. However, relative paths are only allowed in
		 * the current repository.
		 */
		if (*alternate == '.' && !alternate_depth) {
			if ((result = git_str_joinpath(&alternates_path, objects_dir, alternate)) < 0)
				break;
			alternate = git_str_cstr(&alternates_path);
		}

		if ((result = git_odb__add_default_backends(odb, alternate, true, alternate_depth + 1)) < 0)
			break;
	}

	git_str_dispose(&alternates_path);
	git_str_dispose(&alternates_buf);

	return result;
}


// Source: odb.c
// Lines 635-686
