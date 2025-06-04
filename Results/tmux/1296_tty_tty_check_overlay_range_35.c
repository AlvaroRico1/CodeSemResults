tty_check_overlay_range(struct tty *tty, u_int px, u_int py, u_int nx,
    struct overlay_ranges *r)
{
	struct client	*c = tty->client;

	if (c->overlay_check == NULL) {
		r->px[0] = px;
		r->nx[0] = nx;
		r->px[1] = 0;
		r->nx[1] = 0;
		r->px[2] = 0;
		r->nx[2] = 0;
		return;
	}

	c->overlay_check(c, c->overlay_data, px, py, nx, r);
}


// Source: tty.c
// Lines 1329-1345
