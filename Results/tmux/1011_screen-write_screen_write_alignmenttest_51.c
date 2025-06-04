screen_write_alignmenttest(struct screen_write_ctx *ctx)
{
	struct screen		*s = ctx->s;
	struct tty_ctx	 	 ttyctx;
	struct grid_cell       	 gc;
	u_int			 xx, yy;

	memcpy(&gc, &grid_default_cell, sizeof gc);
	utf8_set(&gc.data, 'E');

	for (yy = 0; yy < screen_size_y(s); yy++) {
		for (xx = 0; xx < screen_size_x(s); xx++)
			grid_view_set_cell(s->grid, xx, yy, &gc);
	}

	screen_write_set_cursor(ctx, 0, 0);

	s->rupper = 0;
	s->rlower = screen_size_y(s) - 1;

	screen_write_initctx(ctx, &ttyctx, 1);

	screen_write_collect_clear(ctx, 0, screen_size_y(s) - 1);
	tty_write(tty_cmd_alignmenttest, &ttyctx);
}


// Source: screen-write.c
// Lines 973-997
