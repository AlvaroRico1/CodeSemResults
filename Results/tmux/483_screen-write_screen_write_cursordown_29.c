screen_write_cursordown(struct screen_write_ctx *ctx, u_int ny)
{
	struct screen	*s = ctx->s;
	u_int		 cx = s->cx, cy = s->cy;

	if (ny == 0)
		ny = 1;

	if (cy > s->rlower) {
		/* Below region. */
		if (ny > screen_size_y(s) - 1 - cy)
			ny = screen_size_y(s) - 1 - cy;
	} else {
		/* Above region. */
		if (ny > s->rlower - cy)
			ny = s->rlower - cy;
	}
	if (cx == screen_size_x(s))
	    cx--;
	else if (ny == 0)
		return;

	cy += ny;

	screen_write_set_cursor(ctx, cx, cy);
}


// Source: screen-write.c
// Lines 882-907
