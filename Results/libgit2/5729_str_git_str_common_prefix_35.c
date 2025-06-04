int git_str_common_prefix(git_str *buf, char *const *const strings, size_t count)
{
	size_t i;
	const char *str, *pfx;

	git_str_clear(buf);

	if (!strings || !count)
		return 0;

	/* initialize common prefix to first string */
	if (git_str_sets(buf, strings[0]) < 0)
		return -1;

	/* go through the rest of the strings, truncating to shared prefix */
	for (i = 1; i < count; ++i) {

		for (str = strings[i], pfx = buf->ptr;
		     *str && *str == *pfx;
		     str++, pfx++)
			/* scanning */;

		git_str_truncate(buf, pfx - buf->ptr);

		if (!buf->size)
			break;
	}

	return 0;
}


// Source: str.c
// Lines 1210-1239
