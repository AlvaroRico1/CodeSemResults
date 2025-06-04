layout_cell_is_bottom(struct window *w, struct layout_cell *lc)
{
	struct layout_cell	*next;

	while (lc != w->layout_root) {
		next = lc->parent;
		if (next->type == LAYOUT_TOPBOTTOM &&
		    lc != TAILQ_LAST(&next->cells, layout_cells))
			return (0);
		lc = next;
	}
	return (1);
}


// Source: layout.c
// Lines 259-271
