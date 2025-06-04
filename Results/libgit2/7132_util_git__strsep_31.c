char *git__strsep(char **end, const char *sep)
{
	char *start = *end, *ptr = *end;

	while (*ptr && !strchr(sep, *ptr))
		++ptr;

	if (*ptr) {
		*end = ptr + 1;
		*ptr = '\0';

		return start;
	}

	return NULL;
}


// Source: util.c
// Lines 307-322
