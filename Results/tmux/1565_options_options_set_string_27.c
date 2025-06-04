options_set_string(struct options *oo, const char *name, int append,
    const char *fmt, ...)
{
	struct options_entry	*o;
	va_list			 ap;
	const char		*separator = "";
	char			*s, *value;

	va_start(ap, fmt);
	xvasprintf(&s, fmt, ap);
	va_end(ap);

	o = options_get_only(oo, name);
	if (o != NULL && append && OPTIONS_IS_STRING(o)) {
		if (*name != '@') {
			separator = o->tableentry->separator;
			if (separator == NULL)
				separator = "";
		}
		xasprintf(&value, "%s%s%s", o->value.string, separator, s);
		free(s);
	} else
		value = s;
	if (o == NULL && *name == '@')
		o = options_add(oo, name);
	else if (o == NULL) {
		o = options_default(oo, options_parent_table_entry(oo, name));
		if (o == NULL)
			return (NULL);
	}

	if (!OPTIONS_IS_STRING(o))
		fatalx("option %s is not a string", name);
	free(o->value.string);
	o->value.string = value;
	o->cached = 0;
	return (o);
}


// Source: options.c
// Lines 723-760
