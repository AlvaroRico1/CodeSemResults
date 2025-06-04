static void assert_config_contains_all(git_config_backend *backend,
	struct expected_entry *entries)
{
	int i;

	cl_git_pass(git_config_backend_foreach(backend, contains_all_cb, entries));

	for (i = 0; entries[i].name; i++)
		cl_assert(entries[i].seen);
}


// Source: memory.c
// Lines 51-60
