void test_config_snapshot__snapshot_from_in_memony(void)
{
	const char *configuration = "[section]\nkey = 1\n";
	git_config_backend *backend;
	int i;

	cl_git_pass(git_config_new(&cfg));
	cl_git_pass(git_config_backend_from_string(&backend, configuration, strlen(configuration)));
	cl_git_pass(git_config_add_backend(cfg, backend, 0, NULL, 0));

	cl_git_pass(git_config_snapshot(&snapshot, cfg));
	cl_git_pass(git_config_get_int32(&i, snapshot, "section.key"));
	cl_assert_equal_i(i, 1);
}


// Source: snapshot.c
// Lines 126-139
