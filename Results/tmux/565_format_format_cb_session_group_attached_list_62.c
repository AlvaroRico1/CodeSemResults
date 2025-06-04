format_cb_session_group_attached_list(struct format_tree *ft)
{
	struct session		*s = ft->s, *client_session, *session_loop;
	struct session_group	*sg;
	struct client		*loop;
	struct evbuffer		*buffer;
	int			 size;
	char			*value = NULL;

	if (s == NULL)
		return (NULL);
	sg = session_group_contains(s);
	if (sg == NULL)
		return (NULL);

	buffer = evbuffer_new();
	if (buffer == NULL)
		fatalx("out of memory");

	TAILQ_FOREACH(loop, &clients, entry) {
		client_session = loop->session;
		if (client_session == NULL)
			continue;
		TAILQ_FOREACH(session_loop, &sg->sessions, gentry) {
			if (session_loop == client_session){
				if (EVBUFFER_LENGTH(buffer) > 0)
					evbuffer_add(buffer, ",", 1);
				evbuffer_add_printf(buffer, "%s", loop->name);
			}
		}
	}

	if ((size = EVBUFFER_LENGTH(buffer)) != 0)
		xasprintf(&value, "%.*s", size, EVBUFFER_DATA(buffer));
	evbuffer_free(buffer);
	return (value);
}


// Source: format.c
// Lines 988-1024
