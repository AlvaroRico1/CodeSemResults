screen_write_clearendofscreen(struct screen_write_ctx *ctx, u_int bg)
{
	struct screen	*s = ctx->s;
	struct grid	*gd = s->grid;
	struct tty_ctx	 ttyctx;
	u_int		 sx = screen_size_x(s), sy = screen_size_y(s);

	screen_write_initctx(ctx, &ttyctx, 1);
	ttyctx.bg = bg;

	/* Scroll into history if it is enabled and clearing entire screen. */
	if (s->cx == 0 && s->cy == 0 && (gd->flags & GRID_HISTORY))
		grid_view_clear_history(gd, bg);
	else {
		if (s->cx <= sx - 1)
			grid_view_clear(gd, s->cx, s->cy, sx - s->cx, 1, bg);
		grid_view_clear(gd, 0, s->cy + 1, sx, sy - (s->cy + 1), bg);
	}

	screen_write_collect_clear(ctx, s->cy + 1, sy - (s->cy + 1));
	screen_write_collect_flush(ctx, 0, __func__);
	tty_write(tty_cmd_clearendofscreen, &ttyctx);
}


// Source: screen-write.c
// Lines 1415-1437
