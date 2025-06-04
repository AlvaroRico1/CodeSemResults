screen_redraw_type_of_cell(struct client *c, u_int px, u_int py,
    int pane_status)
{
	struct window	*w = c->session->curw->window;
	u_int		 sx = w->sx, sy = w->sy;
	int		 borders = 0;

	/* Is this outside the window? */
	if (px > sx || py > sy)
		return (CELL_OUTSIDE);

	/*
	 * Construct a bitmask of whether the cells to the left (bit 4), right,
	 * top, and bottom (bit 1) of this cell are borders.
	 */
	if (px == 0 || screen_redraw_cell_border(c, px - 1, py, pane_status))
		borders |= 8;
	if (px <= sx && screen_redraw_cell_border(c, px + 1, py, pane_status))
		borders |= 4;
	if (pane_status == PANE_STATUS_TOP) {
		if (py != 0 &&
		    screen_redraw_cell_border(c, px, py - 1, pane_status))
			borders |= 2;
		if (screen_redraw_cell_border(c, px, py + 1, pane_status))
			borders |= 1;
	} else if (pane_status == PANE_STATUS_BOTTOM) {
		if (py == 0 ||
		    screen_redraw_cell_border(c, px, py - 1, pane_status))
			borders |= 2;
		if (py != sy - 1 &&
		    screen_redraw_cell_border(c, px, py + 1, pane_status))
			borders |= 1;
	} else {
		if (py == 0 ||
		    screen_redraw_cell_border(c, px, py - 1, pane_status))
			borders |= 2;
		if (screen_redraw_cell_border(c, px, py + 1, pane_status))
			borders |= 1;
	}

	/*
	 * Figure out what kind of border this cell is. Only one bit set
	 * doesn't make sense (can't have a border cell with no others
	 * connected).
	 */
	switch (borders) {
	case 15:	/* 1111, left right top bottom */
		return (CELL_JOIN);
	case 14:	/* 1110, left right top */
		return (CELL_BOTTOMJOIN);
	case 13:	/* 1101, left right bottom */
		return (CELL_TOPJOIN);
	case 12:	/* 1100, left right */
		return (CELL_LEFTRIGHT);
	case 11:	/* 1011, left top bottom */
		return (CELL_RIGHTJOIN);
	case 10:	/* 1010, left top */
		return (CELL_BOTTOMRIGHT);
	case 9:		/* 1001, left bottom */
		return (CELL_TOPRIGHT);
	case 7:		/* 0111, right top bottom */
		return (CELL_LEFTJOIN);
	case 6:		/* 0110, right top */
		return (CELL_BOTTOMLEFT);
	case 5:		/* 0101, right bottom */
		return (CELL_TOPLEFT);
	case 3:		/* 0011, top bottom */
		return (CELL_TOPBOTTOM);
	}
	return (CELL_OUTSIDE);
}


// Source: screen-redraw.c
// Lines 204-274
