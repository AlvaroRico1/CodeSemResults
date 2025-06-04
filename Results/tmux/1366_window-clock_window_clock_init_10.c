window_clock_init(struct window_mode_entry *wme,
    __unused struct cmd_find_state *fs, __unused struct args *args)
{
	struct window_pane		*wp = wme->wp;
	struct window_clock_mode_data	*data;
	struct screen			*s;
	struct timeval			 tv = { .tv_sec = 1 };

	wme->data = data = xmalloc(sizeof *data);
	data->tim = time(NULL);

	evtimer_set(&data->timer, window_clock_timer_callback, wme);
	evtimer_add(&data->timer, &tv);

	s = &data->screen;
	screen_init(s, screen_size_x(&wp->base), screen_size_y(&wp->base), 0);
	s->mode &= ~MODE_CURSOR;

	window_clock_draw_screen(wme);

	return (s);
}


// Source: window-clock.c
// Lines 154-175
