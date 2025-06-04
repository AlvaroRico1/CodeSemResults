void test_config_global__open_xdg(void)
{
	git_config *cfg, *xdg, *selected;
	const char *str = "teststring";
	const char *key = "this.variable";
	git_buf buf = {0};

	cl_git_mkfile("xdg/git/config", "# XDG config\n[core]\n  test = 1\n");

	cl_git_pass(git_config_open_default(&cfg));
	cl_git_pass(git_config_open_level(&xdg, cfg, GIT_CONFIG_LEVEL_XDG));
	cl_git_pass(git_config_open_global(&selected, cfg));

	cl_git_pass(git_config_set_string(xdg, key, str));
	cl_git_pass(git_config_get_string_buf(&buf, selected, key));
	cl_assert_equal_s(str, buf.ptr);

	git_buf_dispose(&buf);
	git_config_free(selected);
	git_config_free(xdg);
	git_config_free(cfg);
}


// Source: global.c
// Lines 107-128
