input_exit_apc(struct input_ctx *ictx)
{
	struct screen_write_ctx	*sctx = &ictx->ctx;
	struct window_pane	*wp = ictx->wp;

	if (ictx->flags & INPUT_DISCARD)
		return;
	log_debug("%s: \"%s\"", __func__, ictx->input_buf);

	if (screen_set_title(sctx->s, ictx->input_buf) && wp != NULL) {
		notify_pane("pane-title-changed", wp);
		server_redraw_window_borders(wp->window);
		server_status_window(wp->window);
	}
}


// Source: input.c
// Lines 2351-2365
