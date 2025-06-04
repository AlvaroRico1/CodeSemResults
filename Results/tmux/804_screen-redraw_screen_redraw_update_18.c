screen_redraw_update(struct client *c, int flags)
{
	struct window			*w = c->session->curw->window;
	struct window_pane		*wp;
	struct options			*wo = w->options;
	int				 redraw;
	enum pane_lines			 lines;
	struct screen_redraw_ctx	 ctx;

	if (c->message_string != NULL)
		redraw = status_message_redraw(c);
	else if (c->prompt_string != NULL)
		redraw = status_prompt_redraw(c);
	else
		redraw = status_redraw(c);
	if (!redraw && (~flags & CLIENT_REDRAWSTATUSALWAYS))
		flags &= ~CLIENT_REDRAWSTATUS;

	if (c->overlay_draw != NULL)
		flags |= CLIENT_REDRAWOVERLAY;

	if (options_get_number(wo, "pane-border-status") != PANE_STATUS_OFF) {
		screen_redraw_set_context(c, &ctx);
		lines = options_get_number(wo, "pane-border-lines");
		redraw = 0;
		TAILQ_FOREACH(wp, &w->panes, entry) {
			if (screen_redraw_make_pane_status(c, wp, &ctx, lines))
				redraw = 1;
		}
		if (redraw)
			flags |= CLIENT_REDRAWBORDERS;
	}
	return (flags);
}


// Source: screen-redraw.c
// Lines 480-513
