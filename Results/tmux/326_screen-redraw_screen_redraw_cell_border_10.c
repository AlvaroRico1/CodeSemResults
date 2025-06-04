screen_redraw_cell_border(struct client *c, u_int px, u_int py, int pane_status)
{
	struct window		*w = c->session->curw->window;
	struct window_pane	*wp;

	/* Outside the window? */
	if (px > w->sx || py > w->sy)
		return (0);

	/* On the window border? */
	if (px == w->sx || py == w->sy)
		return (1);

	/* Check all the panes. */
	TAILQ_FOREACH(wp, &w->panes, entry) {
		if (!window_pane_visible(wp))
			continue;
		switch (screen_redraw_pane_border(wp, px, py, pane_status)) {
		case SCREEN_REDRAW_INSIDE:
			return (0);
		case SCREEN_REDRAW_BORDER:
			return (1);
		case SCREEN_REDRAW_OUTSIDE:
			break;
		}
	}

	return (0);
}


// Source: screen-redraw.c
// Lines 172-200
