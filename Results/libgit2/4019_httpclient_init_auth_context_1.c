static const char *init_auth_context(
	git_http_server *server,
	git_vector *challenges,
	git_credential *credentials)
{
	git_http_auth_scheme *scheme;
	const char *challenge;
	int error;

	if (!best_scheme_and_challenge(&scheme, &challenge, challenges, credentials)) {
		git_error_set(GIT_ERROR_HTTP, "could not find appropriate mechanism for credentials");
		return NULL;
	}

	error = scheme->init_context(&server->auth_context, &server->url);

	if (error == GIT_PASSTHROUGH) {
		git_error_set(GIT_ERROR_HTTP, "'%s' authentication is not supported", scheme->name);
		return NULL;
	}

	return challenge;
}


// Source: httpclient.c
// Lines 515-537
