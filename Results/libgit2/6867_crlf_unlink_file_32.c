static int unlink_file(void *payload, git_str *path)
{
	char *fn;

	cl_assert(fn = git_fs_path_basename(path->ptr));

	GIT_UNUSED(payload);

	if (strcmp(fn, ".git"))
		cl_must_pass(p_unlink(path->ptr));

	git__free(fn);
	return 0;
}


// Source: crlf.c
// Lines 16-29
