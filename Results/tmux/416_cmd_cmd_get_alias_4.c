cmd_get_alias(const char *name)
{
	struct options_entry		*o;
	struct options_array_item	*a;
	union options_value		*ov;
	size_t				 wanted, n;
	const char			*equals;

	o = options_get_only(global_options, "command-alias");
	if (o == NULL)
		return (NULL);
	wanted = strlen(name);

	a = options_array_first(o);
	while (a != NULL) {
		ov = options_array_item_value(a);

		equals = strchr(ov->string, '=');
		if (equals != NULL) {
			n = equals - ov->string;
			if (n == wanted && strncmp(name, ov->string, n) == 0)
				return (xstrdup(equals + 1));
		}

		a = options_array_next(a);
	}
	return (NULL);
}


// Source: cmd.c
// Lines 417-444
