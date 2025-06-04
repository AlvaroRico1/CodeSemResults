tty_clear_area(struct tty *tty, const struct grid_cell *defaults, u_int py,
    u_int ny, u_int px, u_int nx, u_int bg)
{
	struct client	*c = tty->client;
	u_int		 yy;
	char		 tmp[64];

	log_debug("%s: %s, %u,%u at %u,%u", __func__, c->name, nx, ny, px, py);

	/* Nothing to clear. */
	if (nx == 0 || ny == 0)
		return;

	/* If genuine BCE is available, can try escape sequences. */
	if (c->overlay_check == NULL && !tty_fake_bce(tty, defaults, bg)) {
		/* Use ED if clearing off the bottom of the terminal. */
		if (px == 0 &&
		    px + nx >= tty->sx &&
		    py + ny >= tty->sy &&
		    tty_term_has(tty->term, TTYC_ED)) {
			tty_cursor(tty, 0, py);
			tty_putcode(tty, TTYC_ED);
			return;
		}

		/*
		 * On VT420 compatible terminals we can use DECFRA if the
		 * background colour isn't default (because it doesn't work
		 * after SGR 0).
		 */
		if ((tty->term->flags & TERM_DECFRA) && !COLOUR_DEFAULT(bg)) {
			xsnprintf(tmp, sizeof tmp, "\033[32;%u;%u;%u;%u$x",
			    py + 1, px + 1, py + ny, px + nx);
			tty_puts(tty, tmp);
			return;
		}

		/* Full lines can be scrolled away to clear them. */
		if (px == 0 &&
		    px + nx >= tty->sx &&
		    ny > 2 &&
		    tty_term_has(tty->term, TTYC_CSR) &&
		    tty_term_has(tty->term, TTYC_INDN)) {
			tty_region(tty, py, py + ny - 1);
			tty_margin_off(tty);
			tty_putcode1(tty, TTYC_INDN, ny);
			return;
		}

		/*
		 * If margins are supported, can just scroll the area off to
		 * clear it.
		 */
		if (nx > 2 &&
		    ny > 2 &&
		    tty_term_has(tty->term, TTYC_CSR) &&
		    tty_use_margin(tty) &&
		    tty_term_has(tty->term, TTYC_INDN)) {
			tty_region(tty, py, py + ny - 1);
			tty_margin(tty, px, px + nx - 1);
			tty_putcode1(tty, TTYC_INDN, ny);
			return;
		}
	}

	/* Couldn't use an escape sequence, loop over the lines. */
	for (yy = py; yy < py + ny; yy++)
		tty_clear_line(tty, defaults, yy, px, nx, bg);
}


// Source: tty.c
// Lines 1176-1244
