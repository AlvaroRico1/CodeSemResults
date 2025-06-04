input_exit_rename(struct input_ctx *ictx)
{
	struct window_pane	*wp = ictx->wp;
	struct options_entry	*o;

	if (wp == NULL)
		return;
	if (ictx->flags & INPUT_DISCARD)
		return;
	if (!options_get_number(ictx->wp->options, "allow-rename"))
		return;
	log_debug("%s: \"%s\"", __func__, ictx->input_buf);

	if (!utf8_isvalid(ictx->input_buf))
		return;

	if (ictx->input_len == 0) {
		o = options_get_only(wp->window->options, "automatic-rename");
		if (o != NULL)
			options_remove_or_default(o, -1, NULL);
		return;
	}
	window_set_name(wp->window, ictx->input_buf);
	options_set_number(wp->window->options, "automatic-rename", 0);
	server_redraw_window_borders(wp->window);
	server_status_window(wp->window);
}


// Source: input.c
// Lines 2380-2406
