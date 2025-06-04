int git_fs_path_normalize_slashes(git_str *out, const char *path)
{
	int error;
	char *p;

	if ((error = git_str_puts(out, path)) < 0)
		return error;

	for (p = out->ptr; *p; p++) {
		if (*p == '\\')
			*p = '/';
	}

	return 0;
}


// Source: fs_path.c
// Lines 1750-1764
