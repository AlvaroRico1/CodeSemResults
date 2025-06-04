tty_clear_pane_line(struct tty *tty, const struct tty_ctx *ctx, u_int py,
    u_int px, u_int nx, u_int bg)
{
	struct client	*c = tty->client;
	u_int		 i, x, rx, ry;

	log_debug("%s: %s, %u at %u,%u", __func__, c->name, nx, px, py);

	if (tty_clamp_line(tty, ctx, px, py, nx, &i, &x, &rx, &ry))
		tty_clear_line(tty, &ctx->defaults, ry, x, rx, bg);
}


// Source: tty.c
// Lines 1100-1110
