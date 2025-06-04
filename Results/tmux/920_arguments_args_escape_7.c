args_escape(const char *s)
{
	static const char	 dquoted[] = " #';${}%";
	static const char	 squoted[] = " \"";
	char			*escaped, *result;
	int			 flags, quotes = 0;

	if (*s == '\0') {
		xasprintf(&result, "''");
		return (result);
	}
	if (s[strcspn(s, dquoted)] != '\0')
		quotes = '"';
	else if (s[strcspn(s, squoted)] != '\0')
		quotes = '\'';

	if (s[0] != ' ' &&
	    s[1] == '\0' &&
	    (quotes != 0 || s[0] == '~')) {
		xasprintf(&escaped, "\\%c", s[0]);
		return (escaped);
	}

	flags = VIS_OCTAL|VIS_CSTYLE|VIS_TAB|VIS_NL;
	if (quotes == '"')
		flags |= VIS_DQ;
	utf8_stravis(&escaped, s, flags);

	if (quotes == '\'')
		xasprintf(&result, "'%s'", escaped);
	else if (quotes == '"') {
		if (*escaped == '~')
			xasprintf(&result, "\"\\%s\"", escaped);
		else
			xasprintf(&result, "\"%s\"", escaped);
	} else {
		if (*escaped == '~')
			xasprintf(&result, "\\%s", escaped);
		else
			result = xstrdup(escaped);
	}
	free(escaped);
	return (result);
}


// Source: arguments.c
// Lines 515-558
