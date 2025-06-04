tty_stop_tty(struct tty *tty)
{
	struct client	*c = tty->client;
	struct winsize	 ws;

	if (!(tty->flags & TTY_STARTED))
		return;
	tty->flags &= ~TTY_STARTED;

	evtimer_del(&tty->start_timer);

	event_del(&tty->timer);
	tty->flags &= ~TTY_BLOCK;

	event_del(&tty->event_in);
	event_del(&tty->event_out);

	/*
	 * Be flexible about error handling and try not kill the server just
	 * because the fd is invalid. Things like ssh -t can easily leave us
	 * with a dead tty.
	 */
	if (ioctl(c->fd, TIOCGWINSZ, &ws) == -1)
		return;
	if (tcsetattr(c->fd, TCSANOW, &tty->tio) == -1)
		return;

	tty_raw(tty, tty_term_string2(tty->term, TTYC_CSR, 0, ws.ws_row - 1));
	if (tty_acs_needed(tty))
		tty_raw(tty, tty_term_string(tty->term, TTYC_RMACS));
	tty_raw(tty, tty_term_string(tty->term, TTYC_SGR0));
	tty_raw(tty, tty_term_string(tty->term, TTYC_RMKX));
	tty_raw(tty, tty_term_string(tty->term, TTYC_CLEAR));
	if (tty->cstyle != SCREEN_CURSOR_DEFAULT) {
		if (tty_term_has(tty->term, TTYC_SE))
			tty_raw(tty, tty_term_string(tty->term, TTYC_SE));
		else if (tty_term_has(tty->term, TTYC_SS))
			tty_raw(tty, tty_term_string1(tty->term, TTYC_SS, 0));
	}
	if (tty->mode & MODE_BRACKETPASTE)
		tty_raw(tty, tty_term_string(tty->term, TTYC_DSBP));
	if (*tty->ccolour != '\0')
		tty_raw(tty, tty_term_string(tty->term, TTYC_CR));

	tty_raw(tty, tty_term_string(tty->term, TTYC_CNORM));
	if (tty_term_has(tty->term, TTYC_KMOUS)) {
		tty_raw(tty, "\033[?1000l\033[?1002l\033[?1003l");
		tty_raw(tty, "\033[?1006l\033[?1005l");
	}

	if (tty->term->flags & TERM_VT100LIKE)
		tty_raw(tty, "\033[?7727l");
	tty_raw(tty, tty_term_string(tty->term, TTYC_DSFCS));
	tty_raw(tty, tty_term_string(tty->term, TTYC_DSEKS));

	if (tty_use_margin(tty))
		tty_raw(tty, tty_term_string(tty->term, TTYC_DSMG));
	tty_raw(tty, tty_term_string(tty->term, TTYC_RMCUP));

	setblocking(c->fd, 1);
}


// Source: tty.c
// Lines 368-428
