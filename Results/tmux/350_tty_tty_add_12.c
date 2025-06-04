tty_add(struct tty *tty, const char *buf, size_t len)
{
	struct client	*c = tty->client;

	if (tty->flags & TTY_BLOCK) {
		tty->discarded += len;
		return;
	}

	evbuffer_add(tty->out, buf, len);
	log_debug("%s: %.*s", c->name, (int)len, buf);
	c->written += len;

	if (tty_log_fd != -1)
		write(tty_log_fd, buf, len);
	if (tty->flags & TTY_STARTED)
		event_add(&tty->event_out, NULL);
}


// Source: tty.c
// Lines 542-559
