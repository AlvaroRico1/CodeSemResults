int git_net_url_apply_redirect(
	git_net_url *url,
	const char *redirect_location,
	bool allow_offsite,
	const char *service_suffix)
{
	git_net_url tmp = GIT_NET_URL_INIT;
	int error = 0;

	GIT_ASSERT(url);
	GIT_ASSERT(redirect_location);

	if (redirect_location[0] == '/') {
		git__free(url->path);

		if ((url->path = git__strdup(redirect_location)) == NULL) {
			error = -1;
			goto done;
		}
	} else {
		git_net_url *original = url;

		if ((error = git_net_url_parse(&tmp, redirect_location)) < 0)
			goto done;

		/* Validate that this is a legal redirection */

		if (original->scheme &&
		    strcmp(original->scheme, tmp.scheme) != 0 &&
		    strcmp(tmp.scheme, "https") != 0) {
			git_error_set(GIT_ERROR_NET, "cannot redirect from '%s' to '%s'",
				original->scheme, tmp.scheme);

			error = -1;
			goto done;
		}

		if (original->host &&
		    !allow_offsite &&
		    git__strcasecmp(original->host, tmp.host) != 0) {
			git_error_set(GIT_ERROR_NET, "cannot redirect from '%s' to '%s'",
				original->host, tmp.host);

			error = -1;
			goto done;
		}

		git_net_url_swap(url, &tmp);
	}


// Source: net.c
// Lines 523-571
