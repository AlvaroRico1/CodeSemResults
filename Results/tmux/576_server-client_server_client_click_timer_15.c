server_client_click_timer(__unused int fd, __unused short events, void *data)
{
	struct client		*c = data;
	struct key_event	*event;

	log_debug("click timer expired");

	if (c->flags & CLIENT_TRIPLECLICK) {
		/*
		 * Waiting for a third click that hasn't happened, so this must
		 * have been a double click.
		 */
		event = xmalloc(sizeof *event);
		event->key = KEYC_DOUBLECLICK;
		memcpy(&event->m, &c->click_event, sizeof event->m);
		if (!server_client_handle_key(c, event))
			free(event);
	}
	c->flags &= ~(CLIENT_DOUBLECLICK|CLIENT_TRIPLECLICK);
}


// Source: server-client.c
// Lines 1797-1816
