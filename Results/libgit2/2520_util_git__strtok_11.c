char *git__strtok(char **end, const char *sep)
{
	char *ptr = *end;

	while (*ptr && strchr(sep, *ptr))
		++ptr;

	if (*ptr) {
		char *start = ptr;
		*end = start + 1;

		while (**end && !strchr(sep, **end))
			++*end;

		if (**end) {
			**end = '\0';
			++*end;
		}

		return start;
	}


// Source: util.c
// Lines 281-301
