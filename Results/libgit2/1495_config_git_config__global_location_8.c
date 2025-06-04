int git_config__global_location(git_str *buf)
{
	const git_str *paths;
	const char *sep, *start;

	if (git_sysdir_get(&paths, GIT_SYSDIR_GLOBAL) < 0)
		return -1;

	/* no paths, so give up */
	if (!paths || !git_str_len(paths))
		return -1;

	/* find unescaped separator or end of string */
	for (sep = start = git_str_cstr(paths); *sep; ++sep) {
		if (*sep == GIT_PATH_LIST_SEPARATOR &&
			(sep <= start || sep[-1] != '\\'))
			break;
	}

	if (git_str_set(buf, start, (size_t)(sep - start)) < 0)
		return -1;

	return git_str_joinpath(buf, buf->ptr, GIT_CONFIG_FILENAME_GLOBAL);
}


// Source: config.c
// Lines 1183-1206
