screen_redraw_draw_pane_status(struct screen_redraw_ctx *ctx)
{
	struct client		*c = ctx->c;
	struct window		*w = c->session->curw->window;
	struct tty		*tty = &c->tty;
	struct window_pane	*wp;
	struct screen		*s;
	u_int			 i, x, width, xoff, yoff, size;

	log_debug("%s: %s @%u", __func__, c->name, w->id);

	TAILQ_FOREACH(wp, &w->panes, entry) {
		if (!window_pane_visible(wp))
			continue;
		s = &wp->status_screen;

		size = wp->status_size;
		if (ctx->pane_status == PANE_STATUS_TOP)
			yoff = wp->yoff - 1;
		else
			yoff = wp->yoff + wp->sy;
		xoff = wp->xoff + 2;

		if (xoff + size <= ctx->ox ||
		    xoff >= ctx->ox + ctx->sx ||
		    yoff < ctx->oy ||
		    yoff >= ctx->oy + ctx->sy)
			continue;

		if (xoff >= ctx->ox && xoff + size <= ctx->ox + ctx->sx) {
			/* All visible. */
			i = 0;
			x = xoff - ctx->ox;
			width = size;
		} else if (xoff < ctx->ox && xoff + size > ctx->ox + ctx->sx) {
			/* Both left and right not visible. */
			i = ctx->ox;
			x = 0;
			width = ctx->sx;
		} else if (xoff < ctx->ox) {
			/* Left not visible. */
			i = ctx->ox - xoff;
			x = 0;
			width = size - i;
		} else {
			/* Right not visible. */
			i = 0;
			x = xoff - ctx->ox;
			width = size - x;
		}

		if (ctx->statustop)
			yoff += ctx->statuslines;
		tty_draw_line(tty, s, i, 0, width, x, yoff - ctx->oy,
		    &grid_default_cell, NULL);
	}
	tty_cursor(tty, 0, 0);
}


// Source: screen-redraw.c
// Lines 419-476
