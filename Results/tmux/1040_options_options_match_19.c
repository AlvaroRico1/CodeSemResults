options_match(const char *s, int *idx, int *ambiguous)
{
	const struct options_table_entry	*oe, *found;
	char					*parsed;
	const char				*name;
	size_t					 namelen;

	parsed = options_parse(s, idx);
	if (parsed == NULL)
		return (NULL);
	if (*parsed == '@') {
		*ambiguous = 0;
		return (parsed);
	}

	name = options_map_name(parsed);
	namelen = strlen(name);

	found = NULL;
	for (oe = options_table; oe->name != NULL; oe++) {
		if (strcmp(oe->name, name) == 0) {
			found = oe;
			break;
		}
		if (strncmp(oe->name, name, namelen) == 0) {
			if (found != NULL) {
				*ambiguous = 1;
				free(parsed);
				return (NULL);
			}
			found = oe;
		}
	}
	free(parsed);
	if (found == NULL) {
		*ambiguous = 0;
		return (NULL);
	}
	return (xstrdup(found->name));
}


// Source: options.c
// Lines 636-675
