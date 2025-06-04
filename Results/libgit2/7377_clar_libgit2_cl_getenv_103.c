char *cl_getenv(const char *name)
{
	git_str out = GIT_STR_INIT;
	int error = git__getenv(&out, name);

	cl_assert(error >= 0 || error == GIT_ENOTFOUND);

	if (error == GIT_ENOTFOUND)
		return NULL;

	if (out.size == 0) {
		char *dup = git__strdup("");
		cl_assert(dup);

		return dup;
	}

	return git_str_detach(&out);
}


// Source: clar_libgit2.c
// Lines 69-87
