void git_fs_path_squash_slashes(git_str *path)
{
	char *p, *q;

	if (path->size == 0)
		return;

	for (p = path->ptr, q = path->ptr; *q; p++, q++) {
		*p = *q;

		while (*q == '/' && *(q+1) == '/') {
			path->size--;
			q++;
		}
	}

	*p = '\0';
}


// Source: fs_path.c
// Lines 360-377
