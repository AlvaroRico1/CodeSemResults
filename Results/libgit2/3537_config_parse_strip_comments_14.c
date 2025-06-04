static int strip_comments(char *line, int in_quotes)
{
	int quote_count = in_quotes, backslash_count = 0;
	char *ptr;

	for (ptr = line; *ptr; ++ptr) {
		if (ptr[0] == '"' && ((ptr > line && ptr[-1] != '\\') || ptr == line))
			quote_count++;

		if ((ptr[0] == ';' || ptr[0] == '#') &&
			(quote_count % 2) == 0 &&
			(backslash_count % 2) == 0) {
			ptr[0] = '\0';
			break;
		}

		if (ptr[0] == '\\')
			backslash_count++;
		else
			backslash_count = 0;
	}

	/* skip any space at the end */
	while (ptr > line && git__isspace(ptr[-1])) {
		ptr--;
	}
	ptr[0] = '\0';

	return quote_count;
}


// Source: config_parse.c
// Lines 33-62
