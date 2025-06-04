screen_write_cursormove(struct screen_write_ctx *ctx, int px, int py,
    int origin)
{
	struct screen	*s = ctx->s;

	if (origin && py != -1 && (s->mode & MODE_ORIGIN)) {
		if ((u_int)py > s->rlower - s->rupper)
			py = s->rlower;
		else
			py += s->rupper;
	}

	if (px != -1 && (u_int)px > screen_size_x(s) - 1)
		px = screen_size_x(s) - 1;
	if (py != -1 && (u_int)py > screen_size_y(s) - 1)
		py = screen_size_y(s) - 1;

	log_debug("%s: from %u,%u to %u,%u", __func__, s->cx, s->cy, px, py);
	screen_write_set_cursor(ctx, px, py);
}


// Source: screen-write.c
// Lines 1264-1283
