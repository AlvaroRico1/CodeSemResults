screen_write_alternateon(struct screen_write_ctx *ctx, struct grid_cell *gc,
    int cursor)
{
	struct tty_ctx		 ttyctx;
	struct window_pane	*wp = ctx->wp;

	if (wp != NULL && !options_get_number(wp->options, "alternate-screen"))
		return;

	screen_write_collect_flush(ctx, 0, __func__);
	screen_alternate_on(ctx->s, gc, cursor);

	screen_write_initctx(ctx, &ttyctx, 1);
	ttyctx.redraw_cb(&ttyctx);
}


// Source: screen-write.c
// Lines 2104-2118
