window_buffer_start_edit(struct window_buffer_modedata *data,
    struct window_buffer_itemdata *item, struct client *c)
{
	struct paste_buffer		*pb;
	const char			*buf;
	size_t				 len;
	struct window_buffer_editdata	*ed;

	if ((pb = paste_get_name(item->name)) == NULL)
		return;
	buf = paste_buffer_data(pb, &len);

	ed = xcalloc(1, sizeof *ed);
	ed->wp_id = data->wp->id;
	ed->name = xstrdup(paste_buffer_name(pb));
	ed->pb = pb;

	if (popup_editor(c, buf, len, window_buffer_edit_close_cb, ed) != 0)
		window_buffer_finish_edit(ed);
}


// Source: window-buffer.c
// Lines 479-498
