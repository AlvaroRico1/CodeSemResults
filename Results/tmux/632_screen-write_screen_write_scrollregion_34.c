screen_write_scrollregion(struct screen_write_ctx *ctx, u_int rupper,
    u_int rlower)
{
	struct screen	*s = ctx->s;

	if (rupper > screen_size_y(s) - 1)
		rupper = screen_size_y(s) - 1;
	if (rlower > screen_size_y(s) - 1)
		rlower = screen_size_y(s) - 1;
	if (rupper >= rlower)	/* cannot be one line */
		return;

	screen_write_collect_flush(ctx, 0, __func__);

	/* Cursor moves to top-left. */
	screen_write_set_cursor(ctx, 0, 0);

	s->rupper = rupper;
	s->rlower = rlower;
}


// Source: screen-write.c
// Lines 1307-1326
