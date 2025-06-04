screen_write_cursorup(struct screen_write_ctx *ctx, u_int ny)
{
	struct screen	*s = ctx->s;
	u_int		 cx = s->cx, cy = s->cy;

	if (ny == 0)
		ny = 1;

	if (cy < s->rupper) {
		/* Above region. */
		if (ny > cy)
			ny = cy;
	} else {
		/* Below region. */
		if (ny > cy - s->rupper)
			ny = cy - s->rupper;
	}
	if (cx == screen_size_x(s))
		cx--;

	cy -= ny;

	screen_write_set_cursor(ctx, cx, cy);
}


// Source: screen-write.c
// Lines 855-878
