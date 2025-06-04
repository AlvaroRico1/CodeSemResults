static void remove_service_suffix(
	git_net_url *url,
	const char *service_suffix)
{
	const char *service_query = strchr(service_suffix, '?');
	size_t full_suffix_len = strlen(service_suffix);
	size_t suffix_len = service_query ?
		(size_t)(service_query - service_suffix) : full_suffix_len;
	size_t path_len = strlen(url->path);
	ssize_t truncate = -1;

	/*
	 * Check for a redirect without query parameters,
	 * like "/newloc/info/refs"'
	 */
	if (suffix_len && path_len >= suffix_len) {
		size_t suffix_offset = path_len - suffix_len;

		if (git__strncmp(url->path + suffix_offset, service_suffix, suffix_len) == 0 &&
		    (!service_query || git__strcmp(url->query, service_query + 1) == 0)) {
			truncate = suffix_offset;
		}
	}

	/*
	 * If we haven't already found where to truncate to remove the
	 * suffix, check for a redirect with query parameters, like
	 * "/newloc/info/refs?service=git-upload-pack"
	 */
	if (truncate < 0 && git__suffixcmp(url->path, service_suffix) == 0)
		truncate = path_len - full_suffix_len;

	/* Ensure we leave a minimum of '/' as the path */
	if (truncate == 0)
		truncate++;

	if (truncate > 0) {
		url->path[truncate] = '\0';

		git__free(url->query);
		url->query = NULL;
	}
}


// Source: net.c
// Lines 479-521
