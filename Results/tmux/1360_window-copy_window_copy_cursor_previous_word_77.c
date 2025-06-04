window_copy_cursor_previous_word(struct window_mode_entry *wme,
    const char *separators, int already)
{
	struct window_copy_mode_data	*data = wme->data;
	struct window			*w = wme->wp->window;
	struct screen			*back_s = data->backing;
	struct grid_reader		 gr;
	u_int				 px, py, oldy, hsize;
	int				 stop_at_eol;

	if (options_get_number(w->options, "mode-keys") == MODEKEY_EMACS)
		stop_at_eol = 1;
	else
		stop_at_eol = 0;

	px = data->cx;
	hsize = screen_hsize(back_s);
	py = hsize + data->cy - data->oy;
	oldy = data->cy;

	grid_reader_start(&gr, back_s->grid, px, py);
	grid_reader_cursor_previous_word(&gr, separators, already, stop_at_eol);
	grid_reader_get_cursor(&gr, &px, &py);
	window_copy_acquire_cursor_up(wme, hsize, data->oy, oldy, px, py);
}


// Source: window-copy.c
// Lines 5189-5213
