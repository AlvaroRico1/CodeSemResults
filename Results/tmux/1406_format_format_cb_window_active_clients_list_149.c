format_cb_window_active_clients_list(struct format_tree *ft)
{
	struct window	*w;
	struct client	*loop;
	struct session	*client_session;
	struct evbuffer	*buffer;
	int		 size;
	char		*value = NULL;

	if (ft->wl == NULL)
		return (NULL);
	w = ft->wl->window;

	buffer = evbuffer_new();
	if (buffer == NULL)
		fatalx("out of memory");

	TAILQ_FOREACH(loop, &clients, entry) {
		client_session = loop->session;
		if (client_session == NULL)
			continue;

		if (w == client_session->curw->window) {
			if (EVBUFFER_LENGTH(buffer) > 0)
				evbuffer_add(buffer, ",", 1);
			evbuffer_add_printf(buffer, "%s", loop->name);
		}
	}

	if ((size = EVBUFFER_LENGTH(buffer)) != 0)
		xasprintf(&value, "%.*s", size, EVBUFFER_DATA(buffer));
	evbuffer_free(buffer);
	return (value);
}


// Source: format.c
// Lines 729-762
