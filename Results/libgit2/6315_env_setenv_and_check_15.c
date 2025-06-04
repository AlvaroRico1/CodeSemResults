static void setenv_and_check(const char *name, const char *value)
{
	char *check;

	cl_git_pass(cl_setenv(name, value));
	check = cl_getenv(name);

	if (value)
		cl_assert_equal_s(value, check);
	else
		cl_assert(check == NULL);

	git__free(check);
}


// Source: env.c
// Lines 68-81
