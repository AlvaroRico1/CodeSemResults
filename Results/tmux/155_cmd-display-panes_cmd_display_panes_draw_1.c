cmd_display_panes_draw(struct client *c, __unused void *data,
    struct screen_redraw_ctx *ctx)
{
	struct window		*w = c->session->curw->window;
	struct window_pane	*wp;

	log_debug("%s: %s @%u", __func__, c->name, w->id);

	TAILQ_FOREACH(wp, &w->panes, entry) {
		if (window_pane_visible(wp))
			cmd_display_panes_draw_pane(ctx, wp);
	}
}


// Source: cmd-display-panes.c
// Lines 198-210
