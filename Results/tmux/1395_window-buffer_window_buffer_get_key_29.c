window_buffer_get_key(void *modedata, void *itemdata, u_int line)
{
	struct window_buffer_modedata	*data = modedata;
	struct window_buffer_itemdata	*item = itemdata;
	struct format_tree		*ft;
	struct session			*s = NULL;
	struct winlink			*wl = NULL;
	struct window_pane		*wp = NULL;
	struct paste_buffer		*pb;
	char				*expanded;
	key_code			 key;

	if (cmd_find_valid_state(&data->fs)) {
		s = data->fs.s;
		wl = data->fs.wl;
		wp = data->fs.wp;
	}
	pb = paste_get_name(item->name);
	if (pb == NULL)
		return KEYC_NONE;

	ft = format_create(NULL, NULL, FORMAT_NONE, 0);
	format_defaults(ft, NULL, NULL, 0, NULL);
	format_defaults(ft, NULL, s, wl, wp);
	format_defaults_paste_buffer(ft, pb);
	format_add(ft, "line", "%u", line);

	expanded = format_expand(ft, data->key_format);
	key = key_string_lookup_string(expanded);
	free(expanded);
	format_free(ft);
	return key;
}


// Source: window-buffer.c
// Lines 292-324
