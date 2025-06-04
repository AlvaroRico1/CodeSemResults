format_strip(const char *s)
{
	char	*out, *cp;
	int	 brackets = 0;

	cp = out = xmalloc(strlen(s) + 1);
	for (; *s != '\0'; s++) {
		if (*s == '#' && s[1] == '{')
			brackets++;
		if (*s == '#' && strchr(",#{}:", s[1]) != NULL) {
			if (brackets != 0)
				*cp++ = *s;
			continue;
		}
		if (*s == '}')
			brackets--;
		*cp++ = *s;
	}
	*cp = '\0';
	return (out);
}


// Source: format.c
// Lines 3430-3450
