void test_config_write__outside_change(void)
{
	int32_t tmp;
	git_config *cfg;
	const char *filename = "config-ext-change";

	cl_git_mkfile(filename, "[old]\nvalue = 5\n");

	cl_git_pass(git_config_open_ondisk(&cfg, filename));

	cl_git_pass(git_config_get_int32(&tmp, cfg, "old.value"));

	/* Change the value on the file itself (simulate external process) */
	cl_git_mkfile(filename, "[old]\nvalue = 6\n");

	cl_git_pass(git_config_set_int32(cfg, "new.value", 7));

	cl_git_pass(git_config_get_int32(&tmp, cfg, "old.value"));
	cl_assert_equal_i(6, tmp);

	git_config_free(cfg);
}


// Source: write.c
// Lines 510-531
