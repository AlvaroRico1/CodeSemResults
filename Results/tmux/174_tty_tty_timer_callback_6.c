tty_timer_callback(__unused int fd, __unused short events, void *data)
{
	struct tty	*tty = data;
	struct client	*c = tty->client;
	struct timeval	 tv = { .tv_usec = TTY_BLOCK_INTERVAL };

	log_debug("%s: %zu discarded", c->name, tty->discarded);

	c->flags |= CLIENT_ALLREDRAWFLAGS;
	c->discarded += tty->discarded;

	if (tty->discarded < TTY_BLOCK_STOP(tty)) {
		tty->flags &= ~TTY_BLOCK;
		tty_invalidate(tty);
		return;
	}
	tty->discarded = 0;
	evtimer_add(&tty->timer, &tv);
}


// Source: tty.c
// Lines 182-200
