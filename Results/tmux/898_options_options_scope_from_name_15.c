options_scope_from_name(struct args *args, int window,
    const char *name, struct cmd_find_state *fs, struct options **oo,
    char **cause)
{
	struct session				*s = fs->s;
	struct winlink				*wl = fs->wl;
	struct window_pane			*wp = fs->wp;
	const char				*target = args_get(args, 't');
	const struct options_table_entry	*oe;
	int					 scope = OPTIONS_TABLE_NONE;

	if (*name == '@')
		return (options_scope_from_flags(args, window, fs, oo, cause));

	for (oe = options_table; oe->name != NULL; oe++) {
		if (strcmp(oe->name, name) == 0)
			break;
	}
	if (oe->name == NULL) {
		xasprintf(cause, "unknown option: %s", name);
		return (OPTIONS_TABLE_NONE);
	}
	switch (oe->scope) {
	case OPTIONS_TABLE_SERVER:
		*oo = global_options;
		scope = OPTIONS_TABLE_SERVER;
		break;
	case OPTIONS_TABLE_SESSION:
		if (args_has(args, 'g')) {
			*oo = global_s_options;
			scope = OPTIONS_TABLE_SESSION;
		} else if (s == NULL && target != NULL)
			xasprintf(cause, "no such session: %s", target);
		else if (s == NULL)
			xasprintf(cause, "no current session");
		else {
			*oo = s->options;
			scope = OPTIONS_TABLE_SESSION;
		}
		break;
	case OPTIONS_TABLE_WINDOW|OPTIONS_TABLE_PANE:
		if (args_has(args, 'p')) {
			if (wp == NULL && target != NULL)
				xasprintf(cause, "no such pane: %s", target);
			else if (wp == NULL)
				xasprintf(cause, "no current pane");
			else {
				*oo = wp->options;
				scope = OPTIONS_TABLE_PANE;
			}
			break;
		}
		/* FALLTHROUGH */
	case OPTIONS_TABLE_WINDOW:
		if (args_has(args, 'g')) {
			*oo = global_w_options;
			scope = OPTIONS_TABLE_WINDOW;
		} else if (wl == NULL && target != NULL)
			xasprintf(cause, "no such window: %s", target);
		else if (wl == NULL)
			xasprintf(cause, "no current window");
		else {
			*oo = wl->window->options;
			scope = OPTIONS_TABLE_WINDOW;
		}
		break;
	}
	return (scope);
}


// Source: options.c
// Lines 784-852
