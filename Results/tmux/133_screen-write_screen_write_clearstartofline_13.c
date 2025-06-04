screen_write_clearstartofline(struct screen_write_ctx *ctx, u_int bg)
{
	struct screen			 *s = ctx->s;
	u_int				 sx = screen_size_x(s);
	struct screen_write_citem	*ci = ctx->item, *before;

	if (s->cx >= sx - 1) {
		screen_write_clearline(ctx, bg);
		return;
	}

	if (s->cx > sx - 1)
		grid_view_clear(s->grid, 0, s->cy, sx, 1, bg);
	else
		grid_view_clear(s->grid, 0, s->cy, s->cx + 1, 1, bg);

	before = screen_write_collect_trim(ctx, s->cy, 0, s->cx + 1, NULL);
	ci->x = 0;
	ci->used = s->cx + 1;
	ci->type = CLEAR;
	ci->bg = bg;
	if (before == NULL)
		TAILQ_INSERT_TAIL(&ctx->s->write_list[s->cy].items, ci, entry);
	else
		TAILQ_INSERT_BEFORE(before, ci, entry);
	ctx->item = screen_write_get_citem();
}


// Source: screen-write.c
// Lines 1234-1260
