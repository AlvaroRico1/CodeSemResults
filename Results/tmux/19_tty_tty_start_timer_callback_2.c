tty_start_timer_callback(__unused int fd, __unused short events, void *data)
{
	struct tty	*tty = data;
	struct client	*c = tty->client;

	log_debug("%s: start timer fired", c->name);
	if ((tty->flags & (TTY_HAVEDA|TTY_HAVEXDA)) == 0)
		tty_update_features(tty);
	tty->flags |= (TTY_HAVEDA|TTY_HAVEXDA);
}


// Source: tty.c
// Lines 289-298
