window_buffer_edit_close_cb(char *buf, size_t len, void *arg)
{
	struct window_buffer_editdata	*ed = arg;
	size_t				 oldlen;
	const char			*oldbuf;
	struct paste_buffer		*pb;
	struct window_pane		*wp;
	struct window_buffer_modedata	*data;
	struct window_mode_entry	*wme;

	if (buf == NULL || len == 0) {
		window_buffer_finish_edit(ed);
		return;
	}

	pb = paste_get_name(ed->name);
	if (pb == NULL || pb != ed->pb) {
		window_buffer_finish_edit(ed);
		return;
	}

	oldbuf = paste_buffer_data(pb, &oldlen);
	if (oldlen != '\0' &&
	    oldbuf[oldlen - 1] != '\n' &&
	    buf[len - 1] == '\n')
		len--;
	if (len != 0)
		paste_replace(pb, buf, len);

	wp = window_pane_find_by_id(ed->wp_id);
	if (wp != NULL) {
		wme = TAILQ_FIRST(&wp->modes);
		if (wme->mode == &window_buffer_mode) {
			data = wme->data;
			mode_tree_build(data->data);
			mode_tree_draw(data->data);
		}
		wp->flags |= PANE_REDRAW;
	}
	window_buffer_finish_edit(ed);
}


// Source: window-buffer.c
// Lines 436-476
