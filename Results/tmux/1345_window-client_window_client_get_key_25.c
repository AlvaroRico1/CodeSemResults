window_client_get_key(void *modedata, void *itemdata, u_int line)
{
	struct window_client_modedata	*data = modedata;
	struct window_client_itemdata	*item = itemdata;
	struct format_tree		*ft;
	char				*expanded;
	key_code			 key;

	ft = format_create(NULL, NULL, FORMAT_NONE, 0);
	format_defaults(ft, item->c, NULL, 0, NULL);
	format_add(ft, "line", "%u", line);

	expanded = format_expand(ft, data->key_format);
	key = key_string_lookup_string(expanded);
	free(expanded);
	format_free(ft);
	return key;
}


// Source: window-client.c
// Lines 268-285
