layout_set_even(struct window *w, enum layout_type type)
{
	struct window_pane	*wp;
	struct layout_cell	*lc, *lcnew;
	u_int			 n, sx, sy;

	layout_print_cell(w->layout_root, __func__, 1);

	/* Get number of panes. */
	n = window_count_panes(w);
	if (n <= 1)
		return;

	/* Free the old root and construct a new. */
	layout_free(w);
	lc = w->layout_root = layout_create_cell(NULL);
	if (type == LAYOUT_LEFTRIGHT) {
		sx = (n * (PANE_MINIMUM + 1)) - 1;
		if (sx < w->sx)
			sx = w->sx;
		sy = w->sy;
	} else {
		sy = (n * (PANE_MINIMUM + 1)) - 1;
		if (sy < w->sy)
			sy = w->sy;
		sx = w->sx;
	}
	layout_set_size(lc, sx, sy, 0, 0);
	layout_make_node(lc, type);

	/* Build new leaf cells. */
	TAILQ_FOREACH(wp, &w->panes, entry) {
		lcnew = layout_create_cell(lc);
		layout_make_leaf(lcnew, wp);
		lcnew->sx = w->sx;
		lcnew->sy = w->sy;
		TAILQ_INSERT_TAIL(&lc->cells, lcnew, entry);
	}

	/* Spread out cells. */
	layout_spread_cell(w, lc);

	/* Fix cell offsets. */
	layout_fix_offsets(w);
	layout_fix_panes(w, NULL);

	layout_print_cell(w->layout_root, __func__, 1);

	window_resize(w, lc->sx, lc->sy, -1, -1);
	notify_window("window-layout-changed", w);
	server_redraw_window(w);
}


// Source: layout-set.c
// Lines 119-170
