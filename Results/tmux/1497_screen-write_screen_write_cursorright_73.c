screen_write_cursorright(struct screen_write_ctx *ctx, u_int nx)
{
	struct screen	*s = ctx->s;
	u_int		 cx = s->cx, cy = s->cy;

	if (nx == 0)
		nx = 1;

	if (nx > screen_size_x(s) - 1 - cx)
		nx = screen_size_x(s) - 1 - cx;
	if (nx == 0)
		return;

	cx += nx;

	screen_write_set_cursor(ctx, cx, cy);
}


// Source: screen-write.c
// Lines 911-927
