status_prompt_complete_list(u_int *size, const char *s, int at_start)
{
	char					**list = NULL;
	const char				**layout, *value, *cp;
	const struct cmd_entry			**cmdent;
	const struct options_table_entry	 *oe;
	size_t					  slen = strlen(s), valuelen;
	struct options_entry			 *o;
	struct options_array_item		 *a;
	const char				 *layouts[] = {
		"even-horizontal", "even-vertical", "main-horizontal",
		"main-vertical", "tiled", NULL
	};

	*size = 0;
	for (cmdent = cmd_table; *cmdent != NULL; cmdent++) {
		if (strncmp((*cmdent)->name, s, slen) == 0) {
			list = xreallocarray(list, (*size) + 1, sizeof *list);
			list[(*size)++] = xstrdup((*cmdent)->name);
		}
		if ((*cmdent)->alias != NULL &&
		    strncmp((*cmdent)->alias, s, slen) == 0) {
			list = xreallocarray(list, (*size) + 1, sizeof *list);
			list[(*size)++] = xstrdup((*cmdent)->alias);
		}
	}
	o = options_get_only(global_options, "command-alias");
	if (o != NULL) {
		a = options_array_first(o);
		while (a != NULL) {
			value = options_array_item_value(a)->string;
			if ((cp = strchr(value, '=')) == NULL)
				goto next;
			valuelen = cp - value;
			if (slen > valuelen || strncmp(value, s, slen) != 0)
				goto next;

			list = xreallocarray(list, (*size) + 1, sizeof *list);
			list[(*size)++] = xstrndup(value, valuelen);

		next:
			a = options_array_next(a);
		}
	}
	if (at_start)
		return (list);

	for (oe = options_table; oe->name != NULL; oe++) {
		if (strncmp(oe->name, s, slen) == 0) {
			list = xreallocarray(list, (*size) + 1, sizeof *list);
			list[(*size)++] = xstrdup(oe->name);
		}
	}
	for (layout = layouts; *layout != NULL; layout++) {
		if (strncmp(*layout, s, slen) == 0) {
			list = xreallocarray(list, (*size) + 1, sizeof *list);
			list[(*size)++] = xstrdup(*layout);
		}
	}
	return (list);
}


// Source: status.c
// Lines 1567-1627
