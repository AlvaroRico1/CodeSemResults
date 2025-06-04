size_t git__unescape(char *str)
{
	char *scan, *pos = str;

	if (!str)
		return 0;

	for (scan = str; *scan; pos++, scan++) {
		if (*scan == '\\' && *(scan + 1) != '\0')
			scan++; /* skip '\' but include next char */
		if (pos != scan)
			*pos = *scan;
	}

	if (pos != scan) {
		*pos = '\0';
	}

	return (pos - str);
}


// Source: util.c
// Lines 655-674
