window_buffer_do_delete(void *modedata, void *itemdata,
    __unused struct client *c, __unused key_code key)
{
	struct window_buffer_modedata	*data = modedata;
	struct window_buffer_itemdata	*item = itemdata;
	struct paste_buffer		*pb;

	if (item == mode_tree_get_current(data->data))
		mode_tree_down(data->data, 0);
	if ((pb = paste_get_name(item->name)) != NULL)
		paste_free(pb);
}


// Source: window-buffer.c
// Lines 404-415
