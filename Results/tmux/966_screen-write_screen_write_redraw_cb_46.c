screen_write_redraw_cb(const struct tty_ctx *ttyctx)
{
	struct window_pane	*wp = ttyctx->arg;

	if (wp != NULL)
		wp->flags |= PANE_REDRAW;
}


// Source: screen-write.c
// Lines 121-127
