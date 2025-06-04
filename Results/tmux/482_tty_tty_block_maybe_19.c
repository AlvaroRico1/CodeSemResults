tty_block_maybe(struct tty *tty)
{
	struct client	*c = tty->client;
	size_t		 size = EVBUFFER_LENGTH(tty->out);
	struct timeval	 tv = { .tv_usec = TTY_BLOCK_INTERVAL };

	if (size < TTY_BLOCK_START(tty))
		return (0);

	if (tty->flags & TTY_BLOCK)
		return (1);
	tty->flags |= TTY_BLOCK;

	log_debug("%s: can't keep up, %zu discarded", c->name, size);

	evbuffer_drain(tty->out, size);
	c->discarded += size;

	tty->discarded = 0;
	evtimer_add(&tty->timer, &tv);
	return (1);
}


// Source: tty.c
// Lines 203-224
