tty_open(struct tty *tty, char **cause)
{
	struct client	*c = tty->client;

	tty->term = tty_term_create(tty, c->term_name, c->term_caps,
	    c->term_ncaps, &c->term_features, cause);
	if (tty->term == NULL) {
		tty_close(tty);
		return (-1);
	}
	tty->flags |= TTY_OPENED;

	tty->flags &= ~(TTY_NOCURSOR|TTY_FREEZE|TTY_BLOCK|TTY_TIMER);

	event_set(&tty->event_in, c->fd, EV_PERSIST|EV_READ,
	    tty_read_callback, tty);
	tty->in = evbuffer_new();
	if (tty->in == NULL)
		fatal("out of memory");

	event_set(&tty->event_out, c->fd, EV_WRITE, tty_write_callback, tty);
	tty->out = evbuffer_new();
	if (tty->out == NULL)
		fatal("out of memory");

	evtimer_set(&tty->timer, tty_timer_callback, tty);

	tty_start_tty(tty);

	tty_keys_build(tty);

	return (0);
}


// Source: tty.c
// Lines 254-286
