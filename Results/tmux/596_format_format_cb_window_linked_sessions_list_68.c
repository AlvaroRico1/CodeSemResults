format_cb_window_linked_sessions_list(struct format_tree *ft)
{
	struct window	*w;
	struct winlink	*wl;
	struct evbuffer	*buffer;
	int		 size;
	char		*value = NULL;

	if (ft->wl == NULL)
		return (NULL);
	w = ft->wl->window;

	buffer = evbuffer_new();
	if (buffer == NULL)
		fatalx("out of memory");

	TAILQ_FOREACH(wl, &w->winlinks, wentry) {
		if (EVBUFFER_LENGTH(buffer) > 0)
			evbuffer_add(buffer, ",", 1);
		evbuffer_add_printf(buffer, "%s", wl->session->name);
	}

	if ((size = EVBUFFER_LENGTH(buffer)) != 0)
		xasprintf(&value, "%.*s", size, EVBUFFER_DATA(buffer));
	evbuffer_free(buffer);
	return (value);
}


// Source: format.c
// Lines 618-644
