layout_close_pane(struct window_pane *wp)
{
	struct window	*w = wp->window;

	/* Remove the cell. */
	layout_destroy_cell(w, wp->layout_cell, &w->layout_root);

	/* Fix pane offsets and sizes. */
	if (w->layout_root != NULL) {
		layout_fix_offsets(w);
		layout_fix_panes(w, NULL);
	}
	notify_window("window-layout-changed", w);
}


// Source: layout.c
// Lines 1037-1050
