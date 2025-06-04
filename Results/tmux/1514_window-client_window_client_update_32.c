window_client_update(struct window_mode_entry *wme)
{
	struct window_client_modedata	*data = wme->data;

	mode_tree_build(data->data);
	mode_tree_draw(data->data);
	data->wp->flags |= PANE_REDRAW;
}


// Source: window-client.c
// Lines 354-361
