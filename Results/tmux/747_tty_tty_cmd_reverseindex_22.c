tty_cmd_reverseindex(struct tty *tty, const struct tty_ctx *ctx)
{
	struct client	*c = tty->client;

	if (ctx->ocy != ctx->orupper)
		return;

	if (ctx->bigger ||
	    (!tty_full_width(tty, ctx) && !tty_use_margin(tty)) ||
	    tty_fake_bce(tty, &ctx->defaults, 8) ||
	    !tty_term_has(tty->term, TTYC_CSR) ||
	    (!tty_term_has(tty->term, TTYC_RI) &&
	    !tty_term_has(tty->term, TTYC_RIN)) ||
	    ctx->sx == 1 ||
	    ctx->sy == 1 ||
	    c->overlay_check != NULL) {
		tty_redraw_region(tty, ctx);
		return;
	}

	tty_default_attributes(tty, &ctx->defaults, ctx->palette, ctx->bg);

	tty_region_pane(tty, ctx, ctx->orupper, ctx->orlower);
	tty_margin_pane(tty, ctx);
	tty_cursor_pane(tty, ctx, ctx->ocx, ctx->orupper);

	if (tty_term_has(tty->term, TTYC_RI))
		tty_putcode(tty, TTYC_RI);
	else
		tty_putcode1(tty, TTYC_RIN, 1);
}


// Source: tty.c
// Lines 1722-1752
