layout_spread_out(struct window_pane *wp)
{
	struct layout_cell	*parent;
	struct window		*w = wp->window;

	parent = wp->layout_cell->parent;
	if (parent == NULL)
		return;

	do {
		if (layout_spread_cell(w, parent)) {
			layout_fix_offsets(w);
			layout_fix_panes(w, NULL);
			break;
		}
	} while ((parent = parent->parent) != NULL);
}


// Source: layout.c
// Lines 1104-1120
