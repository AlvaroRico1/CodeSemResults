tty_update_mode(struct tty *tty, int mode, struct screen *s)
{
	struct client	*c = tty->client;
	int		 changed;

	if (tty->flags & TTY_NOCURSOR)
		mode &= ~MODE_CURSOR;

	changed = mode ^ tty->mode;
	if (log_get_level() != 0 && changed != 0) {
		log_debug("%s: current mode %s", c->name,
		    screen_mode_to_string(tty->mode));
		log_debug("%s: setting mode %s", c->name,
		    screen_mode_to_string(mode));
	}

	tty_update_cursor(tty, mode, changed, s);
	if ((changed & ALL_MOUSE_MODES) &&
	    tty_term_has(tty->term, TTYC_KMOUS)) {
		/*
		 * If the mouse modes have changed, clear any that are set and
		 * apply again. There are differences in how terminals track
		 * the various bits.
		 */
		if (tty->mode & MODE_MOUSE_SGR)
			tty_puts(tty, "\033[?1006l");
		if (tty->mode & MODE_MOUSE_STANDARD)
			tty_puts(tty, "\033[?1000l");
		if (tty->mode & MODE_MOUSE_BUTTON)
			tty_puts(tty, "\033[?1002l");
		if (tty->mode & MODE_MOUSE_ALL)
			tty_puts(tty, "\033[?1003l");
		if (mode & ALL_MOUSE_MODES)
			tty_puts(tty, "\033[?1006h");
		if (mode & MODE_MOUSE_STANDARD)
			tty_puts(tty, "\033[?1000h");
		if (mode & MODE_MOUSE_BUTTON)
			tty_puts(tty, "\033[?1002h");
		if (mode & MODE_MOUSE_ALL)
			tty_puts(tty, "\033[?1003h");
	}
	if (changed & MODE_BRACKETPASTE) {
		if (mode & MODE_BRACKETPASTE)
			tty_putcode(tty, TTYC_ENBP);
		else
			tty_putcode(tty, TTYC_DSBP);
	}
	tty->mode = mode;
}


// Source: tty.c
// Lines 738-786
