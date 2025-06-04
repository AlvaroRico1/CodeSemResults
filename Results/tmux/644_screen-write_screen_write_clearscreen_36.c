screen_write_clearscreen(struct screen_write_ctx *ctx, u_int bg)
{
	struct screen	*s = ctx->s;
	struct tty_ctx	 ttyctx;
	u_int		 sx = screen_size_x(s), sy = screen_size_y(s);

	screen_write_initctx(ctx, &ttyctx, 1);
	ttyctx.bg = bg;

	/* Scroll into history if it is enabled. */
	if (s->grid->flags & GRID_HISTORY)
		grid_view_clear_history(s->grid, bg);
	else
		grid_view_clear(s->grid, 0, 0, sx, sy, bg);

	screen_write_collect_clear(ctx, 0, sy);
	tty_write(tty_cmd_clearscreen, &ttyctx);
}


// Source: screen-write.c
// Lines 1464-1481
