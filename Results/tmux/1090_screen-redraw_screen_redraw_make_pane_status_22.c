screen_redraw_make_pane_status(struct client *c, struct window_pane *wp,
    struct screen_redraw_ctx *rctx, enum pane_lines pane_lines)
{
	struct window		*w = wp->window;
	struct grid_cell	 gc;
	const char		*fmt;
	struct format_tree	*ft;
	char			*expanded;
	int			 pane_status = rctx->pane_status;
	u_int			 width, i, cell_type, px, py;
	struct screen_write_ctx	 ctx;
	struct screen		 old;

	ft = format_create(c, NULL, FORMAT_PANE|wp->id, FORMAT_STATUS);
	format_defaults(ft, c, c->session, c->session->curw, wp);

	if (wp == server_client_get_pane(c))
		style_apply(&gc, w->options, "pane-active-border-style", ft);
	else
		style_apply(&gc, w->options, "pane-border-style", ft);
	fmt = options_get_string(w->options, "pane-border-format");

	expanded = format_expand_time(ft, fmt);
	if (wp->sx < 4)
		wp->status_size = width = 0;
	else
		wp->status_size = width = wp->sx - 4;

	memcpy(&old, &wp->status_screen, sizeof old);
	screen_init(&wp->status_screen, width, 1, 0);
	wp->status_screen.mode = 0;

	screen_write_start(&ctx, &wp->status_screen);

	for (i = 0; i < width; i++) {
		px = wp->xoff + 2 + i;
		if (rctx->pane_status == PANE_STATUS_TOP)
			py = wp->yoff - 1;
		else
			py = wp->yoff + wp->sy;
		cell_type = screen_redraw_type_of_cell(c, px, py, pane_status);
		screen_redraw_border_set(wp, pane_lines, cell_type, &gc);
		screen_write_cell(&ctx, &gc);
	}
	gc.attr &= ~GRID_ATTR_CHARSET;

	screen_write_cursormove(&ctx, 0, 0, 0);
	format_draw(&ctx, &gc, width, expanded, NULL);
	screen_write_stop(&ctx);

	free(expanded);
	format_free(ft);

	if (grid_compare(wp->status_screen.grid, old.grid) == 0) {
		screen_free(&old);
		return (0);
	}
	screen_free(&old);
	return (1);
}


// Source: screen-redraw.c
// Lines 356-415
