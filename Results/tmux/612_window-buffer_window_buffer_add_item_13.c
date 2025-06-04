window_buffer_add_item(struct window_buffer_modedata *data)
{
	struct window_buffer_itemdata	*item;

	data->item_list = xreallocarray(data->item_list, data->item_size + 1,
	    sizeof *data->item_list);
	item = data->item_list[data->item_size++] = xcalloc(1, sizeof *item);
	return (item);
}


// Source: window-buffer.c
// Lines 120-128
