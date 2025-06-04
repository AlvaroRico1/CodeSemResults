screen_write_insertcharacter(struct screen_write_ctx *ctx, u_int nx, u_int bg)
{
	struct screen	*s = ctx->s;
	struct tty_ctx	 ttyctx;

	if (nx == 0)
		nx = 1;

	if (nx > screen_size_x(s) - s->cx)
		nx = screen_size_x(s) - s->cx;
	if (nx == 0)
		return;

	if (s->cx > screen_size_x(s) - 1)
		return;

	screen_write_initctx(ctx, &ttyctx, 0);
	ttyctx.bg = bg;

	grid_view_insert_cells(s->grid, s->cx, s->cy, nx, bg);

	screen_write_collect_flush(ctx, 0, __func__);
	ttyctx.num = nx;
	tty_write(tty_cmd_insertcharacter, &ttyctx);
}


// Source: screen-write.c
// Lines 1001-1025
