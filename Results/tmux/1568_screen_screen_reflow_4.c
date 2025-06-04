screen_reflow(struct screen *s, u_int new_x, u_int *cx, u_int *cy, int cursor)
{
	u_int	wx, wy;

	if (cursor) {
		grid_wrap_position(s->grid, *cx, *cy, &wx, &wy);
		log_debug("%s: cursor %u,%u is %u,%u", __func__, *cx, *cy, wx,
		    wy);
	}

	grid_reflow(s->grid, new_x);

	if (cursor) {
		grid_unwrap_position(s->grid, cx, cy, wx, wy);
		log_debug("%s: new cursor is %u,%u", __func__, *cx, *cy);
	}
	else {
		*cx = 0;
		*cy = s->grid->hsize;
	}
}


// Source: screen.c
// Lines 552-572
