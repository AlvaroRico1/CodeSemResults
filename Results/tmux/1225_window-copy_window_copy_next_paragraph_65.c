window_copy_next_paragraph(struct window_mode_entry *wme)
{
	struct window_copy_mode_data	*data = wme->data;
	struct screen			*s = &data->screen;
	u_int				 maxy, ox, oy;

	oy = screen_hsize(data->backing) + data->cy - data->oy;
	maxy = screen_hsize(data->backing) + screen_size_y(s) - 1;

	while (oy < maxy && window_copy_find_length(wme, oy) == 0)
		oy++;

	while (oy < maxy && window_copy_find_length(wme, oy) > 0)
		oy++;

	ox = window_copy_find_length(wme, oy);
	window_copy_scroll_to(wme, ox, oy, 0);
}


// Source: window-copy.c
// Lines 678-695
