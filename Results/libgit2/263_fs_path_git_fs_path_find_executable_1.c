int git_fs_path_find_executable(git_str *fullpath, const char *executable)
{
#ifdef GIT_WIN32
	git_win32_path fullpath_w, executable_w;
	int error;

	if (git__utf8_to_16(executable_w, GIT_WIN_PATH_MAX, executable) < 0)
		return -1;

	error = git_win32_path_find_executable(fullpath_w, executable_w);

	if (error == 0)
		error = git_str_put_w(fullpath, fullpath_w, wcslen(fullpath_w));

	return error;
#else
	git_str path = GIT_STR_INIT;
	const char *current_dir, *term;
	bool found = false;

	if (git__getenv(&path, "PATH") < 0)
		return -1;

	current_dir = path.ptr;

	while (*current_dir) {
		if (! (term = strchr(current_dir, GIT_PATH_LIST_SEPARATOR)))
			term = strchr(current_dir, '\0');

		git_str_clear(fullpath);
		if (git_str_put(fullpath, current_dir, (term - current_dir)) < 0 ||
		    git_str_putc(fullpath, '/') < 0 ||
		    git_str_puts(fullpath, executable) < 0)
			return -1;

		if (git_fs_path_isfile(fullpath->ptr)) {
			found = true;
			break;
		}

		current_dir = term;

		while (*current_dir == GIT_PATH_LIST_SEPARATOR)
			current_dir++;
	}

	git_str_dispose(&path);

	if (found)
		return 0;

	git_str_clear(fullpath);
	return GIT_ENOTFOUND;
#endif
}


// Source: fs_path.c
// Lines 1864-1918
