static char *sanitized_remote_url(const char *remote_url)
{
	git_net_url url = GIT_NET_URL_INIT;
	char *sanitized = NULL;
	int error;

	if (git_net_url_parse(&url, remote_url) == 0) {
		git_str buf = GIT_STR_INIT;

		git__free(url.username);
		git__free(url.password);
		url.username = url.password = NULL;

		if ((error = git_net_url_fmt(&buf, &url)) < 0)
			goto fallback;

		sanitized = git_str_detach(&buf);
	}

fallback:
	if (!sanitized)
		sanitized = git__strdup(remote_url);

	git_net_url_dispose(&url);
	return sanitized;
}


// Source: fetchhead.c
// Lines 40-65
