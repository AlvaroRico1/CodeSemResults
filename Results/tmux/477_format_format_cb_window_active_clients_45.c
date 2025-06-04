format_cb_window_active_clients(struct format_tree *ft)
{
	struct window	*w;
	struct client	*loop;
	struct session	*client_session;
	u_int		 n = 0;
	char		*value;

	if (ft->wl == NULL)
		return (NULL);
	w = ft->wl->window;

	TAILQ_FOREACH(loop, &clients, entry) {
		client_session = loop->session;
		if (client_session == NULL)
			continue;

		if (w == client_session->curw->window)
			n++;
	}

	xasprintf(&value, "%u", n);
	return (value);
}


// Source: format.c
// Lines 702-725
