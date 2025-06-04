layout_search_by_border(struct layout_cell *lc, u_int x, u_int y)
{
	struct layout_cell	*lcchild, *last = NULL;

	TAILQ_FOREACH(lcchild, &lc->cells, entry) {
		if (x >= lcchild->xoff && x < lcchild->xoff + lcchild->sx &&
		    y >= lcchild->yoff && y < lcchild->yoff + lcchild->sy) {
			/* Inside the cell - recurse. */
			return (layout_search_by_border(lcchild, x, y));
		}

		if (last == NULL) {
			last = lcchild;
			continue;
		}

		switch (lc->type) {
		case LAYOUT_LEFTRIGHT:
			if (x < lcchild->xoff && x >= last->xoff + last->sx)
				return (last);
			break;
		case LAYOUT_TOPBOTTOM:
			if (y < lcchild->yoff && y >= last->yoff + last->sy)
				return (last);
			break;
		case LAYOUT_WINDOWPANE:
			break;
		}

		last = lcchild;
	}

	return (NULL);
}


// Source: layout.c
// Lines 130-163
