layout_resize_pane_grow(struct window *w, struct layout_cell *lc,
    enum layout_type type, int needed, int opposite)
{
	struct layout_cell	*lcadd, *lcremove;
	u_int			 size = 0;

	/* Growing. Always add to the current cell. */
	lcadd = lc;

	/* Look towards the tail for a suitable cell for reduction. */
	lcremove = TAILQ_NEXT(lc, entry);
	while (lcremove != NULL) {
		size = layout_resize_check(w, lcremove, type);
		if (size > 0)
			break;
		lcremove = TAILQ_NEXT(lcremove, entry);
	}

	/* If none found, look towards the head. */
	if (opposite && lcremove == NULL) {
		lcremove = TAILQ_PREV(lc, layout_cells, entry);
		while (lcremove != NULL) {
			size = layout_resize_check(w, lcremove, type);
			if (size > 0)
				break;
			lcremove = TAILQ_PREV(lcremove, layout_cells, entry);
		}
	}
	if (lcremove == NULL)
		return (0);

	/* Change the cells. */
	if (size > (u_int) needed)
		size = needed;
	layout_resize_adjust(w, lcadd, type, size);
	layout_resize_adjust(w, lcremove, type, -size);
	return (size);
}


// Source: layout.c
// Lines 634-671
