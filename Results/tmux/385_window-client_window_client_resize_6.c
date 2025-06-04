window_client_resize(struct window_mode_entry *wme, u_int sx, u_int sy)
{
	struct window_client_modedata	*data = wme->data;

	mode_tree_resize(data->data, sx, sy);
}


// Source: window-client.c
// Lines 346-351
