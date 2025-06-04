screen_write_fast_copy(struct screen_write_ctx *ctx, struct screen *src,
    u_int px, u_int py, u_int nx, u_int ny)
{
	struct screen		*s = ctx->s;
	struct grid		*gd = src->grid;
	struct grid_cell	 gc;
	u_int		 	 xx, yy, cx, cy;

	if (nx == 0 || ny == 0)
		return;

	cy = s->cy;
	for (yy = py; yy < py + ny; yy++) {
		if (yy >= gd->hsize + gd->sy)
			break;
		cx = s->cx;
		for (xx = px; xx < px + nx; xx++) {
			if (xx >= grid_get_line(gd, yy)->cellsize)
				break;
			grid_get_cell(gd, xx, yy, &gc);
			if (xx + gc.data.width > px + nx)
				break;
			grid_view_set_cell(ctx->s->grid, cx, cy, &gc);
			cx++;
		}
		cy++;
	}
}


// Source: screen-write.c
// Lines 556-583
