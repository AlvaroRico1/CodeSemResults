options_parse(const char *name, int *idx)
{
	char	*copy, *cp, *end;

	if (*name == '\0')
		return (NULL);
	copy = xstrdup(name);
	if ((cp = strchr(copy, '[')) == NULL) {
		*idx = -1;
		return (copy);
	}
	end = strchr(cp + 1, ']');
	if (end == NULL || end[1] != '\0' || !isdigit((u_char)end[-1])) {
		free(copy);
		return (NULL);
	}
	if (sscanf(cp, "[%d]", idx) != 1 || *idx < 0) {
		free(copy);
		return (NULL);
	}
	*cp = '\0';
	return (copy);
}


// Source: options.c
// Lines 594-616
