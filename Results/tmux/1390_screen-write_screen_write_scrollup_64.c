screen_write_scrollup(struct screen_write_ctx *ctx, u_int lines, u_int bg)
{
	struct screen	*s = ctx->s;
	struct grid	*gd = s->grid;
	u_int		 i;

	if (lines == 0)
		lines = 1;
	else if (lines > s->rlower - s->rupper + 1)
		lines = s->rlower - s->rupper + 1;

	if (bg != ctx->bg) {
		screen_write_collect_flush(ctx, 1, __func__);
		ctx->bg = bg;
	}

	for (i = 0; i < lines; i++) {
		grid_view_scroll_region_up(gd, s->rupper, s->rlower, bg);
		screen_write_collect_scroll(ctx, bg);
	}
	ctx->scrolled += lines;
}


// Source: screen-write.c
// Lines 1358-1379
