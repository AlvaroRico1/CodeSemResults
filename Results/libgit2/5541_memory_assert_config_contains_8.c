static void assert_config_contains(git_config_backend *backend,
	const char *name, const char *value)
{
	git_config_entry *entry;
	cl_git_pass(git_config_backend_get_string(&entry, backend, name));
	cl_assert_equal_s(entry->value, value);
}


// Source: memory.c
// Lines 17-23
