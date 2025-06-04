layout_cell_is_top(struct window *w, struct layout_cell *lc)
{
	struct layout_cell	*next;

	while (lc != w->layout_root) {
		next = lc->parent;
		if (next->type == LAYOUT_TOPBOTTOM &&
		    lc != TAILQ_FIRST(&next->cells))
			return (0);
		lc = next;
	}
	return (1);
}


// Source: layout.c
// Lines 243-255
