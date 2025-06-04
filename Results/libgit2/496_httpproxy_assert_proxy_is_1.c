static void assert_proxy_is(const char *expected)
{
	git_remote *remote;
	char *proxy;

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
// Lines 43-58
