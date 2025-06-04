window_copy_cursor_previous_word_pos(struct window_mode_entry *wme,
    const char *separators, u_int *ppx, u_int *ppy)
{
	struct window_copy_mode_data	*data = wme->data;
	struct screen			*back_s = data->backing;
	struct grid_reader		 gr;
	u_int				 px, py, hsize;

	px = data->cx;
	hsize = screen_hsize(back_s);
	py = hsize + data->cy - data->oy;

	grid_reader_start(&gr, back_s->grid, px, py);
	grid_reader_cursor_previous_word(&gr, separators, /* already= */ 0,
        /* stop_at_eol= */ 1);
	grid_reader_get_cursor(&gr, &px, &py);
	*ppx = px;
	*ppy = py;
}


// Source: window-copy.c
// Lines 5167-5185
