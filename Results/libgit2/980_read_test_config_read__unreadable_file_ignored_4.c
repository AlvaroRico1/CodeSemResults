void test_config_read__unreadable_file_ignored(void)
{
	git_buf buf = GIT_BUF_INIT;
	git_config *cfg;
	int ret;

	cl_set_cleanup(&clean_test_config, NULL);
	cl_git_mkfile("./testconfig", "[some] var = value\n[some \"OtheR\"] var = value");
	cl_git_pass(p_chmod("./testconfig", 0));

	ret = git_config_open_ondisk(&cfg, "./test/config");
	cl_assert(ret == 0 || ret == GIT_ENOTFOUND);

	git_config_free(cfg);
	git_buf_dispose(&buf);
}


// Source: read.c
// Lines 886-901
