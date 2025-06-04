screen_write_scrolldown(struct screen_write_ctx *ctx, u_int lines, u_int bg)
{
	struct screen	*s = ctx->s;
	struct grid	*gd = s->grid;
	struct tty_ctx	 ttyctx;
	u_int		 i;

	screen_write_initctx(ctx, &ttyctx, 1);
	ttyctx.bg = bg;

	if (lines == 0)
		lines = 1;
	else if (lines > s->rlower - s->rupper + 1)
		lines = s->rlower - s->rupper + 1;

	for (i = 0; i < lines; i++)
		grid_view_scroll_region_down(gd, s->rupper, s->rlower, bg);

	screen_write_collect_flush(ctx, 0, __func__);
	ttyctx.num = lines;
	tty_write(tty_cmd_scrolldown, &ttyctx);
}


// Source: screen-write.c
// Lines 1383-1404
