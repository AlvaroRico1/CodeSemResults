tty_cmd_deletecharacter(struct tty *tty, const struct tty_ctx *ctx)
{
	struct client	*c = tty->client;

	if (ctx->bigger ||
	    !tty_full_width(tty, ctx) ||
	    tty_fake_bce(tty, &ctx->defaults, ctx->bg) ||
	    (!tty_term_has(tty->term, TTYC_DCH) &&
	    !tty_term_has(tty->term, TTYC_DCH1)) ||
	    c->overlay_check != NULL) {
		tty_draw_pane(tty, ctx, ctx->ocy);
		return;
	}

	tty_default_attributes(tty, &ctx->defaults, ctx->palette, ctx->bg);

	tty_cursor_pane(tty, ctx, ctx->ocx, ctx->ocy);

	tty_emulate_repeat(tty, TTYC_DCH, TTYC_DCH1, ctx->num);
}


// Source: tty.c
// Lines 1612-1631
