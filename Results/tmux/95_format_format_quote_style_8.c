format_quote_style(const char *s)
{
	const char	*cp;
	char		*out, *at;

	at = out = xmalloc(strlen(s) * 2 + 1);
	for (cp = s; *cp != '\0'; cp++) {
		if (*cp == '#')
			*at++ = '#';
		*at++ = *cp;
	}
	*at = '\0';
	return (out);
}


// Source: format.c
// Lines 3249-3262
