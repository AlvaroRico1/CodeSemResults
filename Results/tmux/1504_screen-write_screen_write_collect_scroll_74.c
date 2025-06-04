screen_write_collect_scroll(struct screen_write_ctx *ctx, u_int bg)
{
	struct screen			*s = ctx->s;
	struct screen_write_cline	*cl;
	u_int				 y;
	char				*saved;
	struct screen_write_citem	*ci;

	log_debug("%s: at %u,%u (region %u-%u)", __func__, s->cx, s->cy,
	    s->rupper, s->rlower);

	screen_write_collect_clear(ctx, s->rupper, 1);
	saved = ctx->s->write_list[s->rupper].data;
	for (y = s->rupper; y < s->rlower; y++) {
		cl = &ctx->s->write_list[y + 1];
		TAILQ_CONCAT(&ctx->s->write_list[y].items, &cl->items, entry);
		ctx->s->write_list[y].data = cl->data;
	}
	ctx->s->write_list[s->rlower].data = saved;

	ci = screen_write_get_citem();
	ci->x = 0;
	ci->used = screen_size_x(s);
	ci->type = CLEAR;
	ci->bg = bg;
	TAILQ_INSERT_TAIL(&ctx->s->write_list[s->rlower].items, ci, entry);
}


// Source: screen-write.c
// Lines 1603-1629
