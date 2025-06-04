window_copy_pagedown(struct window_mode_entry *wme, int half_page,
    int scroll_exit)
{
	struct window_copy_mode_data	*data = wme->data;
	struct screen			*s = &data->screen;
	u_int				 n, ox, oy, px, py;

	oy = screen_hsize(data->backing) + data->cy - data->oy;
	ox = window_copy_find_length(wme, oy);

	if (data->cx != ox) {
		data->lastcx = data->cx;
		data->lastsx = ox;
	}
	data->cx = data->lastcx;

	n = 1;
	if (screen_size_y(s) > 2) {
		if (half_page)
			n = screen_size_y(s) / 2;
		else
			n = screen_size_y(s) - 2;
	}

	if (data->oy < n) {
		data->oy = 0;
		if (data->cy + (n - data->oy) >= screen_size_y(data->backing))
			data->cy = screen_size_y(data->backing) - 1;
		else
			data->cy += n - data->oy;
	} else
		data->oy -= n;

	if (data->screen.sel == NULL || !data->rectflag) {
		py = screen_hsize(data->backing) + data->cy - data->oy;
		px = window_copy_find_length(wme, py);
		if ((data->cx >= data->lastsx && data->cx != px) ||
		    data->cx > px)
			window_copy_cursor_end_of_line(wme);
	}

	if (scroll_exit && data->oy == 0)
		return (1);
	if (data->searchmark != NULL && !data->timeout)
		window_copy_search_marks(wme, NULL, data->searchregex, 1);
	window_copy_update_selection(wme, 1, 0);
	window_copy_redraw_screen(wme);
	return (0);
}


// Source: window-copy.c
// Lines 610-658
