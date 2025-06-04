static size_t strcspn_escaped(const char *s, const char *stop)
{
	const char *i;

	for (i = s; *i; i++) {
		/* skip the escaped character */
		if (i[0] == '\\' && i[1]) {
			i++;
			continue;
		}

		if (strchr(stop, *i))
			break;
	}
	return i - s;
}


// Source: pathspec.c
// Lines 117-132
