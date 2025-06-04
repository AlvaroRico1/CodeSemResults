window_copy_cursor_jump_to_back(struct window_mode_entry *wme)
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
	grid_reader_cursor_left(&gr, 0);
	grid_reader_cursor_left(&gr, 0);
	if (grid_reader_cursor_jump_back(&gr, data->jumpchar)) {
		grid_reader_cursor_right(&gr, 1, 0);
		grid_reader_get_cursor(&gr, &px, &py);
		window_copy_acquire_cursor_up(wme, hsize, data->oy, oldy, px,
		    py);
	}
}


// Source: window-copy.c
// Lines 5062-5083
