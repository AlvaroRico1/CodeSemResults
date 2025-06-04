screen_write_backspace(struct screen_write_ctx *ctx)
{
	struct screen		*s = ctx->s;
	struct grid_line	*gl;
	u_int			 cx = s->cx, cy = s->cy;

	if (cx == 0) {
		if (cy == 0)
			return;
		gl = grid_get_line(s->grid, s->grid->hsize + cy - 1);
		if (gl->flags & GRID_LINE_WRAPPED) {
			cy--;
			cx = screen_size_x(s) - 1;
		}
	} else
		cx--;

	screen_write_set_cursor(ctx, cx, cy);
}


// Source: screen-write.c
// Lines 951-969
