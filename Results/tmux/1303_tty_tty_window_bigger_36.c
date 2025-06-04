tty_window_bigger(struct tty *tty)
{
	struct client	*c = tty->client;
	struct window	*w = c->session->curw->window;

	return (tty->sx < w->sx || tty->sy - status_line_size(c) < w->sy);
}


// Source: tty.c
// Lines 818-824
