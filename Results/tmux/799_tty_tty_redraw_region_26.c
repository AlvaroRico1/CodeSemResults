tty_redraw_region(struct tty *tty, const struct tty_ctx *ctx)
{
	struct client		*c = tty->client;
	u_int			 i;

	/*
	 * If region is large, schedule a redraw. In most cases this is likely
	 * to be followed by some more scrolling.
	 */
	if (tty_large_region(tty, ctx)) {
		log_debug("%s: %s large redraw", __func__, c->name);
		ctx->redraw_cb(ctx);
		return;
	}

	for (i = ctx->orupper; i <= ctx->orlower; i++)
		tty_draw_pane(tty, ctx, i);
}


// Source: tty.c
// Lines 973-990
