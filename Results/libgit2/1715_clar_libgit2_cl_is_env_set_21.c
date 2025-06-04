bool cl_is_env_set(const char *name)
{
	char *env = cl_getenv(name);
	bool result = (env != NULL);
	git__free(env);
	return result;
}


// Source: clar_libgit2.c
// Lines 89-95
