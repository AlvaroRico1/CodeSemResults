static size_t unescape_spaces(char *str)
{
	char *scan, *pos = str;
	bool escaped = false;

	if (!str)
		return 0;

	for (scan = str; *scan; scan++) {
		if (!escaped && *scan == '\\') {
			escaped = true;
			continue;
		}

		/* Only insert the escape character for escaped non-spaces */
		if (escaped && !git__isspace(*scan))
			*pos++ = '\\';

		*pos++ = *scan;
		escaped = false;
	}

	if (pos != scan)
		*pos = '\0';

	return (pos - str);
}


// Source: attr_file.c
// Lines 672-698
