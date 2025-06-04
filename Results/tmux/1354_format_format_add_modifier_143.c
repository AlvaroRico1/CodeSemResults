format_add_modifier(struct format_modifier **list, u_int *count,
    const char *c, size_t n, char **argv, int argc)
{
	struct format_modifier *fm;

	*list = xreallocarray(*list, (*count) + 1, sizeof **list);
	fm = &(*list)[(*count)++];

	memcpy(fm->modifier, c, n);
	fm->modifier[n] = '\0';
	fm->size = n;

	fm->argv = argv;
	fm->argc = argc;
}


// Source: format.c
// Lines 3519-3533
