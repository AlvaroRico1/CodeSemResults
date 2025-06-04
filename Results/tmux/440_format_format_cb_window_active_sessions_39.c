format_cb_window_active_sessions(struct format_tree *ft)
{
	struct window	*w;
	struct winlink	*wl;
	u_int		 n = 0;
	char		*value;

	if (ft->wl == NULL)
		return (NULL);
	w = ft->wl->window;

	TAILQ_FOREACH(wl, &w->winlinks, wentry) {
		if (wl->session->curw == wl)
			n++;
	}

	xasprintf(&value, "%u", n);
	return (value);
}


// Source: format.c
// Lines 648-666
