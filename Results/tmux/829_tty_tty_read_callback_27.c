tty_read_callback(__unused int fd, __unused short events, void *data)
{
	struct tty	*tty = data;
	struct client	*c = tty->client;
	const char	*name = c->name;
	size_t		 size = EVBUFFER_LENGTH(tty->in);
	int		 nread;

	nread = evbuffer_read(tty->in, c->fd, -1);
	if (nread == 0 || nread == -1) {
		if (nread == 0)
			log_debug("%s: read closed", name);
		else
			log_debug("%s: read error: %s", name, strerror(errno));
		event_del(&tty->event_in);
		server_client_lost(tty->client);
		return;
	}
	log_debug("%s: read %d bytes (already %zu)", name, nread, size);

	while (tty_keys_next(tty))
		;
}


// Source: tty.c
// Lines 157-179
