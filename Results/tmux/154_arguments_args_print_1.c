args_print(struct args *args)
{
	size_t			 len;
	char			*buf;
	u_int			 i, j;
	struct args_entry	*entry;
	struct args_value	*value;

	len = 1;
	buf = xcalloc(1, len);

	/* Process the flags first. */
	RB_FOREACH(entry, args_tree, &args->tree) {
		if (!TAILQ_EMPTY(&entry->values))
			continue;

		if (*buf == '\0')
			args_print_add(&buf, &len, "-");
		for (j = 0; j < entry->count; j++)
			args_print_add(&buf, &len, "%c", entry->flag);
	}

	/* Then the flags with arguments. */
	RB_FOREACH(entry, args_tree, &args->tree) {
		TAILQ_FOREACH(value, &entry->values, entry) {
			if (*buf != '\0')
				args_print_add(&buf, &len, " -%c", entry->flag);
			else
				args_print_add(&buf, &len, "-%c", entry->flag);
			args_print_add_value(&buf, &len, value);
		}
	}

	/* And finally the argument vector. */
	for (i = 0; i < args->count; i++)
		args_print_add_value(&buf, &len, &args->values[i]);

	return (buf);
}


// Source: arguments.c
// Lines 473-511
