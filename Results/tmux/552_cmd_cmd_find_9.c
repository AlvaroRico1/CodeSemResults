cmd_find(const char *name, char **cause)
{
	const struct cmd_entry	**loop, *entry, *found = NULL;
	int			  ambiguous;
	char			  s[8192];

	ambiguous = 0;
	for (loop = cmd_table; *loop != NULL; loop++) {
		entry = *loop;
		if (entry->alias != NULL && strcmp(entry->alias, name) == 0) {
			ambiguous = 0;
			found = entry;
			break;
		}

		if (strncmp(entry->name, name, strlen(name)) != 0)
			continue;
		if (found != NULL)
			ambiguous = 1;
		found = entry;

		if (strcmp(entry->name, name) == 0)
			break;
	}
	if (ambiguous)
		goto ambiguous;
	if (found == NULL) {
		xasprintf(cause, "unknown command: %s", name);
		return (NULL);
	}
	return (found);

ambiguous:
	*s = '\0';
	for (loop = cmd_table; *loop != NULL; loop++) {
		entry = *loop;
		if (strncmp(entry->name, name, strlen(name)) != 0)
			continue;
		if (strlcat(s, entry->name, sizeof s) >= sizeof s)
			break;
		if (strlcat(s, ", ", sizeof s) >= sizeof s)
			break;
	}
	s[strlen(s) - 2] = '\0';
	xasprintf(cause, "ambiguous command: %s, could be: %s", name, s);
	return (NULL);
}


// Source: cmd.c
// Lines 448-494
