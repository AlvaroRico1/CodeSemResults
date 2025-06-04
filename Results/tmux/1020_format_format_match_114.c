format_match(struct format_modifier *fm, const char *pattern, const char *text)
{
	const char	*s = "";
	regex_t		 r;
	int		 flags = 0;

	if (fm->argc >= 1)
		s = fm->argv[0];
	if (strchr(s, 'r') == NULL) {
		if (strchr(s, 'i') != NULL)
			flags |= FNM_CASEFOLD;
		if (fnmatch(pattern, text, flags) != 0)
			return (xstrdup("0"));
	} else {
		flags = REG_EXTENDED|REG_NOSUB;
		if (strchr(s, 'i') != NULL)
			flags |= REG_ICASE;
		if (regcomp(&r, pattern, flags) != 0)
			return (xstrdup("0"));
		if (regexec(&r, text, 0, NULL, 0) != 0) {
			regfree(&r);
			return (xstrdup("0"));
		}
		regfree(&r);
	}
	return (xstrdup("1"));
}


// Source: format.c
// Lines 3659-3685
