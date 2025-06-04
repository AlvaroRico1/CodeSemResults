window_buffer_init(struct window_mode_entry *wme, struct cmd_find_state *fs,
    struct args *args)
{
	struct window_pane		*wp = wme->wp;
	struct window_buffer_modedata	*data;
	struct screen			*s;

	wme->data = data = xcalloc(1, sizeof *data);
	data->wp = wp;
	cmd_find_copy_state(&data->fs, fs);

	if (args == NULL || !args_has(args, 'F'))
		data->format = xstrdup(WINDOW_BUFFER_DEFAULT_FORMAT);
	else
		data->format = xstrdup(args_get(args, 'F'));
	if (args == NULL || !args_has(args, 'K'))
		data->key_format = xstrdup(WINDOW_BUFFER_DEFAULT_KEY_FORMAT);
	else
		data->key_format = xstrdup(args_get(args, 'K'));
	if (args == NULL || args_count(args) == 0)
		data->command = xstrdup(WINDOW_BUFFER_DEFAULT_COMMAND);
	else
		data->command = xstrdup(args_string(args, 0));

	data->data = mode_tree_start(wp, args, window_buffer_build,
	    window_buffer_draw, window_buffer_search, window_buffer_menu, NULL,
	    window_buffer_get_key, data, window_buffer_menu_items,
	    window_buffer_sort_list, nitems(window_buffer_sort_list), &s);
	mode_tree_zoom(data->data, args);

	mode_tree_build(data->data);
	mode_tree_draw(data->data);

	return (s);
}


// Source: window-buffer.c
// Lines 327-361
