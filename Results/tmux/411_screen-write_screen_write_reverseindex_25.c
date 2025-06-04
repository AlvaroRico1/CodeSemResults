screen_write_reverseindex(struct screen_write_ctx *ctx, u_int bg)
{
	struct screen	*s = ctx->s;
	struct tty_ctx	 ttyctx;

	if (s->cy == s->rupper) {
		grid_view_scroll_region_down(s->grid, s->rupper, s->rlower, bg);
		screen_write_collect_flush(ctx, 0, __func__);

		screen_write_initctx(ctx, &ttyctx, 1);
		ttyctx.bg = bg;

		tty_write(tty_cmd_reverseindex, &ttyctx);
	} else if (s->cy > 0)
		screen_write_set_cursor(ctx, -1, s->cy - 1);

}


// Source: screen-write.c
// Lines 1287-1303
