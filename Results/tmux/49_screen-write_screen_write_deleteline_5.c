screen_write_deleteline(struct screen_write_ctx *ctx, u_int ny, u_int bg)
{
	struct screen	*s = ctx->s;
	struct grid	*gd = s->grid;
	struct tty_ctx	 ttyctx;

	if (ny == 0)
		ny = 1;

	if (s->cy < s->rupper || s->cy > s->rlower) {
		if (ny > screen_size_y(s) - s->cy)
			ny = screen_size_y(s) - s->cy;
		if (ny == 0)
			return;

		screen_write_initctx(ctx, &ttyctx, 1);
		ttyctx.bg = bg;

		grid_view_delete_lines(gd, s->cy, ny, bg);

		screen_write_collect_flush(ctx, 0, __func__);
		ttyctx.num = ny;
		tty_write(tty_cmd_deleteline, &ttyctx);
		return;
	}

	if (ny > s->rlower + 1 - s->cy)
		ny = s->rlower + 1 - s->cy;
	if (ny == 0)
		return;

	screen_write_initctx(ctx, &ttyctx, 1);
	ttyctx.bg = bg;

	if (s->cy < s->rupper || s->cy > s->rlower)
		grid_view_delete_lines(gd, s->cy, ny, bg);
	else
		grid_view_delete_lines_region(gd, s->rlower, s->cy, ny, bg);

	screen_write_collect_flush(ctx, 0, __func__);
	ttyctx.num = ny;
	tty_write(tty_cmd_deleteline, &ttyctx);
}


// Source: screen-write.c
// Lines 1132-1174
