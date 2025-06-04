window_customize_find_user_options(struct options *oo, const char ***list,
    u_int *size)
{
	struct options_entry	*o;
	const char		*name;
	u_int			 i;

	o = options_first(oo);
	while (o != NULL) {
		name = options_name(o);
		if (*name != '@') {
			o = options_next(o);
			continue;
		}
		for (i = 0; i < *size; i++) {
			if (strcmp((*list)[i], name) == 0)
				break;
		}
		if (i != *size) {
			o = options_next(o);
			continue;
		}
		*list = xreallocarray(*list, (*size) + 1, sizeof **list);
		(*list)[(*size)++] = name;

		o = options_next(o);
	}
}


// Source: window-customize.c
// Lines 341-368
