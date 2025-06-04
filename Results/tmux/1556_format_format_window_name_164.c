format_window_name(struct format_expand_state *es, const char *fmt)
{
	struct format_tree	*ft = es->ft;
	char			*name;
	struct winlink		*wl;

	if (ft->s == NULL) {
		format_log(es, "window name but no session");
		return (NULL);
	}

	name = format_expand1(es, fmt);
	RB_FOREACH(wl, winlinks, &ft->s->windows) {
		if (strcmp(wl->window->name, name) == 0) {
			free(name);
			return (xstrdup("1"));
		}
	}
	free(name);
	return (xstrdup("0"));
}


// Source: format.c
// Lines 3775-3795
