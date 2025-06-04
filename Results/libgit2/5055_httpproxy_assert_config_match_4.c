static void assert_config_match(const char *config, const char *expected)
{
	git_remote *remote;
	char *proxy;

	if (config)
		cl_repo_set_string(repo, config, expected);

	cl_git_pass(git_remote_lookup(&remote, repo, "lg2"));
	cl_git_pass(git_remote__http_proxy(&proxy, remote, &url));

	if (expected)
		cl_assert_equal_s(proxy, expected);
	else
		cl_assert_equal_p(proxy, expected);

	git_remote_free(remote);
	git__free(proxy);
}


// Source: httpproxy.c
// Lines 60-78
