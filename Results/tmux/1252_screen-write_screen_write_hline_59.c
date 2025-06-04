screen_write_hline(struct screen_write_ctx *ctx, u_int nx, int left, int right)
{
	struct screen		*s = ctx->s;
	struct grid_cell	 gc;
	u_int			 cx, cy, i;

	cx = s->cx;
	cy = s->cy;

	memcpy(&gc, &grid_default_cell, sizeof gc);
	gc.attr |= GRID_ATTR_CHARSET;

	screen_write_putc(ctx, &gc, left ? 't' : 'q');
	for (i = 1; i < nx - 1; i++)
		screen_write_putc(ctx, &gc, 'q');
	screen_write_putc(ctx, &gc, right ? 'u' : 'q');

	screen_write_set_cursor(ctx, cx, cy);
}


// Source: screen-write.c
// Lines 587-605
