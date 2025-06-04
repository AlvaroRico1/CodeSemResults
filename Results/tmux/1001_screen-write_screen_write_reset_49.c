screen_write_reset(struct screen_write_ctx *ctx)
{
	struct screen	*s = ctx->s;

	screen_reset_tabs(s);
	screen_write_scrollregion(ctx, 0, screen_size_y(s) - 1);

	s->mode = MODE_CURSOR | MODE_WRAP;

	screen_write_clearscreen(ctx, 8);
	screen_write_set_cursor(ctx, 0, 0);
}


// Source: screen-write.c
// Lines 314-325
