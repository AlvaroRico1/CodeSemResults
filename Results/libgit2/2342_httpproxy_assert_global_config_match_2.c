static void assert_global_config_match(const char *config, const char *expected)
{
	git_remote *remote;
	char *proxy;
	git_config* cfg;

	if (config) {
		cl_git_pass(git_config_open_default(&cfg));
		git_config_set_string(cfg, config, expected);
		git_config_free(cfg);
	}

	cl_git_pass(git_remote_create_detached(&remote, "https://github.com/libgit2/libgit2"));
	cl_git_pass(git_remote__http_proxy(&proxy, remote, &url));

	if (expected)
		cl_assert_equal_s(proxy, expected);
	else
		cl_assert_equal_p(proxy, expected);

	git_remote_free(remote);
	git__free(proxy);
}


// Source: httpproxy.c
// Lines 109-131
