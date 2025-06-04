window_copy_cursor_right(struct window_mode_entry *wme, int all)
{
	struct window_copy_mode_data	*data = wme->data;
	struct screen			*back_s = data->backing;
	struct grid_reader		 gr;
	u_int				 px, py, oldy, hsize;

	px = data->cx;
	hsize = screen_hsize(back_s);
	py = hsize + data->cy - data->oy;
	oldy = data->cy;

	grid_reader_start(&gr, back_s->grid, px, py);
	grid_reader_cursor_right(&gr, 1, all);
	grid_reader_get_cursor(&gr, &px, &py);
	window_copy_acquire_cursor_down(wme, hsize, screen_size_y(back_s),
	    data->oy, oldy, px, py, 0);
}


// Source: window-copy.c
// Lines 4837-4854
