window_copy_previous_paragraph(struct window_mode_entry *wme)
{
	struct window_copy_mode_data	*data = wme->data;
	u_int				 oy;

	oy = screen_hsize(data->backing) + data->cy - data->oy;

	while (oy > 0 && window_copy_find_length(wme, oy) == 0)
		oy--;

	while (oy > 0 && window_copy_find_length(wme, oy) > 0)
		oy--;

	window_copy_scroll_to(wme, 0, oy, 0);
}


// Source: window-copy.c
// Lines 661-675
