tty_cmd_scrollup(struct tty *tty, const struct tty_ctx *ctx)
{
	struct client	*c = tty->client;
	u_int		 i;

	if (ctx->bigger ||
	    (!tty_full_width(tty, ctx) && !tty_use_margin(tty)) ||
	    tty_fake_bce(tty, &ctx->defaults, 8) ||
	    !tty_term_has(tty->term, TTYC_CSR) ||
	    ctx->sx == 1 ||
	    ctx->sy == 1 ||
	    c->overlay_check != NULL) {
		tty_redraw_region(tty, ctx);
		return;
	}

	tty_default_attributes(tty, &ctx->defaults, ctx->palette, ctx->bg);

	tty_region_pane(tty, ctx, ctx->orupper, ctx->orlower);
	tty_margin_pane(tty, ctx);

	if (ctx->num == 1 || !tty_term_has(tty->term, TTYC_INDN)) {
		if (!tty_use_margin(tty))
			tty_cursor(tty, 0, tty->rlower);
		else
			tty_cursor(tty, tty->rright, tty->rlower);
		for (i = 0; i < ctx->num; i++)
			tty_putc(tty, '\n');
	} else {
		if (tty->cy == UINT_MAX)
			tty_cursor(tty, 0, 0);
		else
			tty_cursor(tty, 0, tty->cy);
		tty_putcode1(tty, TTYC_INDN, ctx->num);
	}
}


// Source: tty.c
// Lines 1797-1832
