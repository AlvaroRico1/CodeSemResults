layout_resize_pane(struct window_pane *wp, enum layout_type type, int change,
    int opposite)
{
	struct layout_cell	*lc, *lcparent;

	lc = wp->layout_cell;

	/* Find next parent of the same type. */
	lcparent = lc->parent;
	while (lcparent != NULL && lcparent->type != type) {
		lc = lcparent;
		lcparent = lc->parent;
	}
	if (lcparent == NULL)
		return;

	/* If this is the last cell, move back one. */
	if (lc == TAILQ_LAST(&lcparent->cells, layout_cells))
		lc = TAILQ_PREV(lc, layout_cells, entry);

	layout_resize_layout(wp->window, lc, type, change, opposite);
}


// Source: layout.c
// Lines 609-630
