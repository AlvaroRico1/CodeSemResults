tty_draw_pane(struct tty *tty, const struct tty_ctx *ctx, u_int py)
{
	struct screen	*s = ctx->s;
	u_int		 nx = ctx->sx, i, x, rx, ry;

	log_debug("%s: %s %u %d", __func__, tty->client->name, py, ctx->bigger);

	if (!ctx->bigger) {
		tty_draw_line(tty, s, 0, py, nx, ctx->xoff, ctx->yoff + py,
		    &ctx->defaults, ctx->palette);
		return;
	}
	if (tty_clamp_line(tty, ctx, 0, py, nx, &i, &x, &rx, &ry)) {
		tty_draw_line(tty, s, i, py, rx, x, ry, &ctx->defaults,
		    ctx->palette);
	}
}


// Source: tty.c
// Lines 1258-1274
