int git_net_url_parse(git_net_url *url, const char *given)
{
	struct http_parser_url u = {0};
	bool has_scheme, has_host, has_port, has_path, has_query, has_userinfo;
	git_str scheme = GIT_STR_INIT,
		host = GIT_STR_INIT,
		port = GIT_STR_INIT,
		path = GIT_STR_INIT,
		username = GIT_STR_INIT,
		password = GIT_STR_INIT,
		query = GIT_STR_INIT;
	int error = GIT_EINVALIDSPEC;

	if (http_parser_parse_url(given, strlen(given), false, &u)) {
		git_error_set(GIT_ERROR_NET, "malformed URL '%s'", given);
		goto done;
	}

	has_scheme = !!(u.field_set & (1 << UF_SCHEMA));
	has_host = !!(u.field_set & (1 << UF_HOST));
	has_port = !!(u.field_set & (1 << UF_PORT));
	has_path = !!(u.field_set & (1 << UF_PATH));
	has_query = !!(u.field_set & (1 << UF_QUERY));
	has_userinfo = !!(u.field_set & (1 << UF_USERINFO));

	if (has_scheme) {
		const char *url_scheme = given + u.field_data[UF_SCHEMA].off;
		size_t url_scheme_len = u.field_data[UF_SCHEMA].len;
		git_str_put(&scheme, url_scheme, url_scheme_len);
		git__strntolower(scheme.ptr, scheme.size);
	} else {
		git_error_set(GIT_ERROR_NET, "malformed URL '%s'", given);
		goto done;
	}


// Source: net.c
// Lines 96-129
