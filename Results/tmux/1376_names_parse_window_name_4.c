parse_window_name(const char *in)
{
	char	*copy, *name, *ptr;

	name = copy = xstrdup(in);
	if (*name == '"')
		name++;
	name[strcspn(name, "\"")] = '\0';

	if (strncmp(name, "exec ", (sizeof "exec ") - 1) == 0)
		name = name + (sizeof "exec ") - 1;

	while (*name == ' ' || *name == '-')
		name++;
	if ((ptr = strchr(name, ' ')) != NULL)
		*ptr = '\0';

	if (*name != '\0') {
		ptr = name + strlen(name) - 1;
		while (ptr > name &&
		    !isalnum((u_char)*ptr) &&
		    !ispunct((u_char)*ptr))
			*ptr-- = '\0';
	}

	if (*name == '/')
		name = basename(name);
	name = xstrdup(name);
	free(copy);
	return (name);
}


// Source: names.c
// Lines 142-172
