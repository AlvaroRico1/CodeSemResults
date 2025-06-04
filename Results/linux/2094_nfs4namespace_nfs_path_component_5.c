static char *nfs_path_component(const char *nfspath, const char *end)
{
	char *p;

	if (*nfspath == '[') {
		/* parse [] escaped IPv6 addrs */
		p = strchr(nfspath, ']');
		if (p != NULL && ++p < end && *p == ':')
			return p + 1;
	} else {
		/* otherwise split on first colon */
		p = strchr(nfspath, ':');
		if (p != NULL && p < end)
			return p + 1;
	}
	return NULL;
}


// Source: nfs4namespace.c
// Lines 63-79
