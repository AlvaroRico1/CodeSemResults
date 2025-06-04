screen_redraw_draw_status(struct screen_redraw_ctx *ctx)
{
	struct client	*c = ctx->c;
	struct window	*w = c->session->curw->window;
	struct tty	*tty = &c->tty;
	struct screen	*s = c->status.active;
	u_int		 i, y;

	log_debug("%s: %s @%u", __func__, c->name, w->id);

	if (ctx->statustop)
		y = 0;
	else
		y = c->tty.sy - ctx->statuslines;
	for (i = 0; i < ctx->statuslines; i++) {
		tty_draw_line(tty, s, 0, i, UINT_MAX, 0, y + i,
		    &grid_default_cell, NULL);
	}
}


// Source: screen-redraw.c
// Lines 736-754
