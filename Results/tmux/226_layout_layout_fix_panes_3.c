layout_fix_panes(struct window *w, struct window_pane *skip)
{
	struct window_pane	*wp;
	struct layout_cell	*lc;
	int			 status;

	status = options_get_number(w->options, "pane-border-status");
	TAILQ_FOREACH(wp, &w->panes, entry) {
		if ((lc = wp->layout_cell) == NULL || wp == skip)
			continue;

		wp->xoff = lc->xoff;
		wp->yoff = lc->yoff;

		if (layout_add_border(w, lc, status)) {
			if (status == PANE_STATUS_TOP)
				wp->yoff++;
			window_pane_resize(wp, lc->sx, lc->sy - 1);
		} else
			window_pane_resize(wp, lc->sx, lc->sy);
	}
}


// Source: layout.c
// Lines 289-310
