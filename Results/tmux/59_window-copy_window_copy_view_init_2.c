window_copy_view_init(struct window_mode_entry *wme,
    __unused struct cmd_find_state *fs, __unused struct args *args)
{
	struct window_pane		*wp = wme->wp;
	struct window_copy_mode_data	*data;
	struct screen			*base = &wp->base;
	struct screen			*s;

	data = window_copy_common_init(wme);
	data->viewmode = 1;

	data->backing = s = xmalloc(sizeof *data->backing);
	screen_init(s, screen_size_x(base), screen_size_y(base), UINT_MAX);
	data->mx = data->cx;
	data->my = screen_hsize(data->backing) + data->cy - data->oy;
	data->showmark = 0;

	return (&data->screen);
}


// Source: window-copy.c
// Lines 464-482
