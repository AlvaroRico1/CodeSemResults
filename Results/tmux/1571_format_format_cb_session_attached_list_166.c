format_cb_session_attached_list(struct format_tree *ft)
{
	struct session	*s = ft->s;
	struct client	*loop;
	struct evbuffer	*buffer;
	int		 size;
	char		*value = NULL;

	if (s == NULL)
		return (NULL);

	buffer = evbuffer_new();
	if (buffer == NULL)
		fatalx("out of memory");

	TAILQ_FOREACH(loop, &clients, entry) {
		if (loop->session == s) {
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
// Lines 510-537
