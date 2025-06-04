void cl_git_write2file(
	const char *path, const char *content, size_t content_len,
	int flags, unsigned int mode)
{
	int fd;
	cl_assert(path && content);
	cl_assert((fd = p_open(path, flags, mode)) >= 0);
	if (!content_len)
		content_len = strlen(content);
	cl_must_pass(p_write(fd, content, content_len));
	cl_must_pass(p_close(fd));
}


// Source: clar_libgit2.c
// Lines 41-52
