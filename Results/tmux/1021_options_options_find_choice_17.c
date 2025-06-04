options_find_choice(const struct options_table_entry *oe, const char *value,
    char **cause)
{
	const char	**cp;
	int		  n = 0, choice = -1;

	for (cp = oe->choices; *cp != NULL; cp++) {
		if (strcmp(*cp, value) == 0)
			choice = n;
		n++;
	}
	if (choice == -1) {
		xasprintf(cause, "unknown value: %s", value);
		return (-1);
	}
	return (choice);
}


// Source: options.c
// Lines 993-1009
