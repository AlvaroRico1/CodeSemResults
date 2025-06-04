options_array_assign(struct options_entry *o, const char *s, char **cause)
{
	const char	*separator;
	char		*copy, *next, *string;
	u_int		 i;

	separator = o->tableentry->separator;
	if (separator == NULL)
		separator = " ,";
	if (*separator == '\0') {
		if (*s == '\0')
			return (0);
		for (i = 0; i < UINT_MAX; i++) {
			if (options_array_item(o, i) == NULL)
				break;
		}
		return (options_array_set(o, i, s, 0, cause));
	}

	if (*s == '\0')
		return (0);
	copy = string = xstrdup(s);
	while ((next = strsep(&string, separator)) != NULL) {
		if (*next == '\0')
			continue;
		for (i = 0; i < UINT_MAX; i++) {
			if (options_array_item(o, i) == NULL)
				break;
		}
		if (i == UINT_MAX)
			break;
		if (options_array_set(o, i, next, 0, cause) != 0) {
			free(copy);
			return (-1);
		}
	}
	free(copy);
	return (0);
}


// Source: options.c
// Lines 499-537
