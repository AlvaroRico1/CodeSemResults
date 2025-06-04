screen_write_collect_add(struct screen_write_ctx *ctx,
    const struct grid_cell *gc)
{
	struct screen			*s = ctx->s;
	struct screen_write_citem	*ci;
	u_int				 sx = screen_size_x(s);
	int				 collect;

	/*
	 * Don't need to check that the attributes and whatnot are still the
	 * same - input_parse will end the collection when anything that isn't
	 * a plain character is encountered.
	 */

	collect = 1;
	if (gc->data.width != 1 || gc->data.size != 1 || *gc->data.data >= 0x7f)
		collect = 0;
	else if (gc->attr & GRID_ATTR_CHARSET)
		collect = 0;
	else if (~s->mode & MODE_WRAP)
		collect = 0;
	else if (s->mode & MODE_INSERT)
		collect = 0;
	else if (s->sel != NULL)
		collect = 0;
	if (!collect) {
		screen_write_collect_end(ctx);
		screen_write_collect_flush(ctx, 0, __func__);
		screen_write_cell(ctx, gc);
		return;
	}

	if (s->cx > sx - 1 || ctx->item->used > sx - 1 - s->cx)
		screen_write_collect_end(ctx);
	ci = ctx->item; /* may have changed */

	if (s->cx > sx - 1) {
		log_debug("%s: wrapped at %u,%u", __func__, s->cx, s->cy);
		ci->wrapped = 1;
		screen_write_linefeed(ctx, 1, 8);
		screen_write_set_cursor(ctx, 0, -1);
	}

	if (ci->used == 0)
		memcpy(&ci->gc, gc, sizeof ci->gc);
	if (ctx->s->write_list[s->cy].data == NULL)
		ctx->s->write_list[s->cy].data = xmalloc(screen_size_x(ctx->s));
	ctx->s->write_list[s->cy].data[s->cx + ci->used++] = gc->data.data[0];
}


// Source: screen-write.c
// Lines 1749-1797
