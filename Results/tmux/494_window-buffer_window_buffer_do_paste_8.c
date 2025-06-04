window_buffer_do_paste(void *modedata, void *itemdata, struct client *c,
    __unused key_code key)
{
	struct window_buffer_modedata	*data = modedata;
	struct window_buffer_itemdata	*item = itemdata;

	if (paste_get_name(item->name) != NULL)
		mode_tree_run_command(c, NULL, data->command, item->name);
}


// Source: window-buffer.c
// Lines 418-426
