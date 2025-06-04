window_client_add_item(struct window_client_modedata *data)
{
	struct window_client_itemdata	*item;

	data->item_list = xreallocarray(data->item_list, data->item_size + 1,
	    sizeof *data->item_list);
	item = data->item_list[data->item_size++] = xcalloc(1, sizeof *item);
	return (item);
}


// Source: window-client.c
// Lines 109-117
