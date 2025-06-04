session_check_name(const char *name)
{
	char	*copy, *cp, *new_name;

	if (*name == '\0')
		return (NULL);
	copy = xstrdup(name);
	for (cp = copy; *cp != '\0'; cp++) {
		if (*cp == ':' || *cp == '.')
			*cp = '_';
	}
	utf8_stravis(&new_name, copy, VIS_OCTAL|VIS_CSTYLE|VIS_TAB|VIS_NL);
	free(copy);
	return (new_name);
}


// Source: session.c
// Lines 237-251
