options_scope_from_flags(struct args *args, int window,
    struct cmd_find_state *fs, struct options **oo, char **cause)
{
	struct session		*s = fs->s;
	struct winlink		*wl = fs->wl;
	struct window_pane	*wp = fs->wp;
	const char		*target = args_get(args, 't');

	if (args_has(args, 's')) {
		*oo = global_options;
		return (OPTIONS_TABLE_SERVER);
	}

	if (args_has(args, 'p')) {
		if (wp == NULL) {
			if (target != NULL)
				xasprintf(cause, "no such pane: %s", target);
			else
				xasprintf(cause, "no current pane");
			return (OPTIONS_TABLE_NONE);
		}
		*oo = wp->options;
		return (OPTIONS_TABLE_PANE);
	} else if (window || args_has(args, 'w')) {
		if (args_has(args, 'g')) {
			*oo = global_w_options;
			return (OPTIONS_TABLE_WINDOW);
		}
		if (wl == NULL) {
			if (target != NULL)
				xasprintf(cause, "no such window: %s", target);
			else
				xasprintf(cause, "no current window");
			return (OPTIONS_TABLE_NONE);
		}
		*oo = wl->window->options;
		return (OPTIONS_TABLE_WINDOW);
	} else {
		if (args_has(args, 'g')) {
			*oo = global_s_options;
			return (OPTIONS_TABLE_SESSION);
		}
		if (s == NULL) {
			if (target != NULL)
				xasprintf(cause, "no such session: %s", target);
			else
				xasprintf(cause, "no current session");
			return (OPTIONS_TABLE_NONE);
		}
		*oo = s->options;
		return (OPTIONS_TABLE_SESSION);
	}
}


// Source: options.c
// Lines 855-907
