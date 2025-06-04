screen_write_preview(struct screen_write_ctx *ctx, struct screen *src, u_int nx,
    u_int ny)
{
	struct screen		*s = ctx->s;
	struct grid_cell	 gc;
	u_int			 cx, cy, px, py;

	cx = s->cx;
	cy = s->cy;

	/*
	 * If the cursor is on, pick the area around the cursor, otherwise use
	 * the top left.
	 */
	if (src->mode & MODE_CURSOR) {
		px = src->cx;
		if (px < nx / 3)
			px = 0;
		else
			px = px - nx / 3;
		if (px + nx > screen_size_x(src)) {
			if (nx > screen_size_x(src))
				px = 0;
			else
				px = screen_size_x(src) - nx;
		}
		py = src->cy;
		if (py < ny / 3)
			py = 0;
		else
			py = py - ny / 3;
		if (py + ny > screen_size_y(src)) {
			if (ny > screen_size_y(src))
				py = 0;
			else
				py = screen_size_y(src) - ny;
		}
	} else {
		px = 0;
		py = 0;
	}

	screen_write_fast_copy(ctx, src, px, src->grid->hsize + py, nx, ny);

	if (src->mode & MODE_CURSOR) {
		grid_view_get_cell(src->grid, src->cx, src->cy, &gc);
		gc.attr |= GRID_ATTR_REVERSE;
		screen_write_set_cursor(ctx, cx + (src->cx - px),
		    cy + (src->cy - py));
		screen_write_cell(ctx, &gc);
	}
}


// Source: screen-write.c
// Lines 776-827
