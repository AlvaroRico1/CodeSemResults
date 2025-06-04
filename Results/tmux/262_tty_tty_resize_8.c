tty_resize(struct tty *tty)
{
	struct client	*c = tty->client;
	struct winsize	 ws;
	u_int		 sx, sy, xpixel, ypixel;

	if (ioctl(c->fd, TIOCGWINSZ, &ws) != -1) {
		sx = ws.ws_col;
		if (sx == 0) {
			sx = 80;
			xpixel = 0;
		} else
			xpixel = ws.ws_xpixel / sx;
		sy = ws.ws_row;
		if (sy == 0) {
			sy = 24;
			ypixel = 0;
		} else
			ypixel = ws.ws_ypixel / sy;
	} else {
		sx = 80;
		sy = 24;
		xpixel = 0;
		ypixel = 0;
	}
	log_debug("%s: %s now %ux%u (%ux%u)", __func__, c->name, sx, sy,
	    xpixel, ypixel);
	tty_set_size(tty, sx, sy, xpixel, ypixel);
	tty_invalidate(tty);
}


// Source: tty.c
// Lines 116-145
