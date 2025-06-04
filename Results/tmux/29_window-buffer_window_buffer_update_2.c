window_buffer_update(struct window_mode_entry *wme)
{
	struct window_buffer_modedata	*data = wme->data;

	mode_tree_build(data->data);
	mode_tree_draw(data->data);
	data->wp->flags |= PANE_REDRAW;
}


// Source: window-buffer.c
// Lines 394-401
