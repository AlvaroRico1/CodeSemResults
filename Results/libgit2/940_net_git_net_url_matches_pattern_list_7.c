bool git_net_url_matches_pattern_list(
	git_net_url *url,
	const char *pattern_list)
{
	const char *pattern, *pattern_end, *sep;

	for (pattern = pattern_list;
	     pattern && *pattern;
	     pattern = sep ? sep + 1 : NULL) {
		sep = strchr(pattern, ',');
		pattern_end = sep ? sep : strchr(pattern, '\0');

		if (matches_pattern(url, pattern, (pattern_end - pattern)))
			return true;
	}

	return false;
}


// Source: net.c
// Lines 715-732
