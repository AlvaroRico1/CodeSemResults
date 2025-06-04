window_copy_cursor_next_word_end_pos(struct window_mode_entry *wme,
    const char *separators, u_int *ppx, u_int *ppy)
{
	struct window_pane		*wp = wme->wp;
	struct window_copy_mode_data	*data = wme->data;
	struct options			*oo = wp->window->options;
	struct screen			*back_s = data->backing;
	struct grid_reader		 gr;
	u_int				 px, py, hsize;

	px = data->cx;
	hsize = screen_hsize(back_s);
	py =  hsize + data->cy - data->oy;

	grid_reader_start(&gr, back_s->grid, px, py);
	if (options_get_number(oo, "mode-keys") == MODEKEY_VI) {
		if (!grid_reader_in_set(&gr, WHITESPACE))
			grid_reader_cursor_right(&gr, 0, 0);
		grid_reader_cursor_next_word_end(&gr, separators);
		grid_reader_cursor_left(&gr, 1);
	} else
		grid_reader_cursor_next_word_end(&gr, separators);
	grid_reader_get_cursor(&gr, &px, &py);
	*ppx = px;
	*ppy = py;
}


// Source: window-copy.c
// Lines 5108-5133
