static char *sandbox_path(git_str *buf, const char *suffix)
{
	char *path = p_realpath(clar_sandbox_path(), NULL);
	cl_assert(path);
	cl_git_pass(git_str_attach(buf, path, 0));
	cl_git_pass(git_str_joinpath(buf, buf->ptr, suffix));
	return buf->ptr;
}


// Source: conditionals.c
// Lines 53-60
