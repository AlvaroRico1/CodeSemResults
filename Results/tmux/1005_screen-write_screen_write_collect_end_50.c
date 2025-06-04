screen_write_collect_end(struct screen_write_ctx *ctx)
{
	struct screen			*s = ctx->s;
	struct screen_write_citem	*ci = ctx->item, *before;
	struct screen_write_cline	*cl = &s->write_list[s->cy];
	struct grid_cell		 gc;
	u_int				 xx;
	int				 wrapped = ci->wrapped;

	if (ci->used == 0)
		return;

	before = screen_write_collect_trim(ctx, s->cy, s->cx, ci->used,
	    &wrapped);
	ci->x = s->cx;
	ci->wrapped = wrapped;
	if (before == NULL)
		TAILQ_INSERT_TAIL(&cl->items, ci, entry);
	else
		TAILQ_INSERT_BEFORE(before, ci, entry);
	ctx->item = screen_write_get_citem();

	log_debug("%s: %u %.*s (at %u,%u)", __func__, ci->used,
	    (int)ci->used, cl->data + ci->x, s->cx, s->cy);

	if (s->cx != 0) {
		for (xx = s->cx; xx > 0; xx--) {
			grid_view_get_cell(s->grid, xx, s->cy, &gc);
			if (~gc.flags & GRID_FLAG_PADDING)
				break;
			grid_view_set_cell(s->grid, xx, s->cy,
			    &grid_default_cell);
		}
		if (gc.data.width > 1) {
			grid_view_set_cell(s->grid, xx, s->cy,
			    &grid_default_cell);
		}
	}

	grid_view_set_cells(s->grid, s->cx, s->cy, &ci->gc, cl->data + ci->x,
	    ci->used);
	screen_write_set_cursor(ctx, s->cx + ci->used, -1);

	for (xx = s->cx; xx < screen_size_x(s); xx++) {
		grid_view_get_cell(s->grid, xx, s->cy, &gc);
		if (~gc.flags & GRID_FLAG_PADDING)
			break;
		grid_view_set_cell(s->grid, xx, s->cy, &grid_default_cell);
	}
}


// Source: screen-write.c
// Lines 1696-1745
