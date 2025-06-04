tty_cmd_insertline(struct tty *tty, const struct tty_ctx *ctx)
{
	struct client	*c = tty->client;

	if (ctx->bigger ||
	    !tty_full_width(tty, ctx) ||
	    tty_fake_bce(tty, &ctx->defaults, ctx->bg) ||
	    !tty_term_has(tty->term, TTYC_CSR) ||
	    !tty_term_has(tty->term, TTYC_IL1) ||
	    ctx->sx == 1 ||
	    ctx->sy == 1 ||
	    c->overlay_check != NULL) {
		tty_redraw_region(tty, ctx);
		return;
	}

	tty_default_attributes(tty, &ctx->defaults, ctx->palette, ctx->bg);

	tty_region_pane(tty, ctx, ctx->orupper, ctx->orlower);
	tty_margin_off(tty);
	tty_cursor_pane(tty, ctx, ctx->ocx, ctx->ocy);

	tty_emulate_repeat(tty, TTYC_IL, TTYC_IL1, ctx->num);
	tty->cx = tty->cy = UINT_MAX;
}


// Source: tty.c
// Lines 1642-1666
