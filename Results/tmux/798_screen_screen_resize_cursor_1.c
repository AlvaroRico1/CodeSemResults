screen_resize_cursor(struct screen *s, u_int sx, u_int sy, int reflow,
    int eat_empty, int cursor)
{
	u_int	cx = s->cx, cy = s->grid->hsize + s->cy;

	if (s->write_list != NULL)
		screen_write_free_list(s);

	log_debug("%s: new size %ux%u, now %ux%u (cursor %u,%u = %u,%u)",
	    __func__, sx, sy, screen_size_x(s), screen_size_y(s), s->cx, s->cy,
	    cx, cy);

	if (sx < 1)
		sx = 1;
	if (sy < 1)
		sy = 1;

	if (sx != screen_size_x(s)) {
		s->grid->sx = sx;
		screen_reset_tabs(s);
	} else
		reflow = 0;

	if (sy != screen_size_y(s))
		screen_resize_y(s, sy, eat_empty, &cy);

	if (reflow)
		screen_reflow(s, sx, &cx, &cy, cursor);

	if (cy >= s->grid->hsize) {
		s->cx = cx;
		s->cy = cy - s->grid->hsize;
	} else {
		s->cx = 0;
		s->cy = 0;
	}

	log_debug("%s: cursor finished at %u,%u = %u,%u", __func__, s->cx,
	    s->cy, cx, cy);

	if (s->write_list != NULL)
		screen_write_make_list(s);
}


// Source: screen.c
// Lines 256-298
