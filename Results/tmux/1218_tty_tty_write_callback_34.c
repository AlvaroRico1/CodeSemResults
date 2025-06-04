tty_write_callback(__unused int fd, __unused short events, void *data)
{
	struct tty	*tty = data;
	struct client	*c = tty->client;
	size_t		 size = EVBUFFER_LENGTH(tty->out);
	int		 nwrite;

	nwrite = evbuffer_write(tty->out, c->fd);
	if (nwrite == -1)
		return;
	log_debug("%s: wrote %d bytes (of %zu)", c->name, nwrite, size);

	if (c->redraw > 0) {
		if ((size_t)nwrite >= c->redraw)
			c->redraw = 0;
		else
			c->redraw -= nwrite;
		log_debug("%s: waiting for redraw, %zu bytes left", c->name,
		    c->redraw);
	} else if (tty_block_maybe(tty))
		return;

	if (EVBUFFER_LENGTH(tty->out) != 0)
		event_add(&tty->event_out, NULL);
}


// Source: tty.c
// Lines 227-251
