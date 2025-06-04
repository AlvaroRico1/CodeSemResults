static bool matches_pattern(
	git_net_url *url,
	const char *pattern,
	size_t pattern_len)
{
	const char *domain, *port = NULL, *colon;
	size_t host_len, domain_len, port_len = 0, wildcard = 0;

	GIT_UNUSED(url);
	GIT_UNUSED(pattern);

	if (!pattern_len)
		return false;
	else if (pattern_len == 1 && pattern[0] == '*')
		return true;
	else if (pattern_len > 1 && pattern[0] == '*' && pattern[1] == '.')
		wildcard = 2;
	else if (pattern[0] == '.')
		wildcard = 1;

	domain = pattern + wildcard;
	domain_len = pattern_len - wildcard;

	if ((colon = memchr(domain, ':', domain_len)) != NULL) {
		domain_len = colon - domain;
		port = colon + 1;
		port_len = pattern_len - wildcard - domain_len - 1;
	}

	/* A pattern's port *must* match if it's specified */
	if (port_len && git__strlcmp(url->port, port, port_len) != 0)
		return false;

	/* No wildcard?  Host must match exactly. */
	if (!wildcard)
		return !git__strlcmp(url->host, domain, domain_len);

	/* Wildcard: ensure there's (at least) a suffix match */
	if ((host_len = strlen(url->host)) < domain_len ||
	    memcmp(url->host + (host_len - domain_len), domain, domain_len))
		return false;

	/* The pattern is *.domain and the host is simply domain */
	if (host_len == domain_len)
		return true;

	/* The pattern is *.domain and the host is foo.domain */
	return (url->host[host_len - domain_len - 1] == '.');
}


// Source: net.c
// Lines 660-708
