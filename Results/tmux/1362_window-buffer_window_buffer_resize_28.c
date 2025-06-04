window_buffer_resize(struct window_mode_entry *wme, u_int sx, u_int sy)
{
	struct window_buffer_modedata	*data = wme->data;

	mode_tree_resize(data->data, sx, sy);
}


// Source: window-buffer.c
// Lines 386-391
