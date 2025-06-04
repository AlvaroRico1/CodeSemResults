window_copy_common_init(struct window_mode_entry *wme)
{
	struct window_pane		*wp = wme->wp;
	struct window_copy_mode_data	*data;
	struct screen			*base = &wp->base;

	wme->data = data = xcalloc(1, sizeof *data);

	data->cursordrag = CURSORDRAG_NONE;
	data->lineflag = LINE_SEL_NONE;
	data->selflag = SEL_CHAR;

	if (wp->searchstr != NULL) {
		data->searchtype = WINDOW_COPY_SEARCHUP;
		data->searchregex = wp->searchregex;
		data->searchstr = xstrdup(wp->searchstr);
	} else {
		data->searchtype = WINDOW_COPY_OFF;
		data->searchregex = 0;
		data->searchstr = NULL;
	}
	data->searchx = data->searchy = data->searcho = -1;
	data->searchall = 1;

	data->jumptype = WINDOW_COPY_OFF;
	data->jumpchar = NULL;

	screen_init(&data->screen, screen_size_x(base), screen_size_y(base), 0);
	data->modekeys = options_get_number(wp->window->options, "mode-keys");

	evtimer_set(&data->dragtimer, window_copy_scroll_timer, wme);

	return (data);
}


// Source: window-copy.c
// Lines 387-420
