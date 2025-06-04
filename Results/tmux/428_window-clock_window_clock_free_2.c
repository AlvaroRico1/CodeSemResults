window_clock_free(struct window_mode_entry *wme)
{
	struct window_clock_mode_data	*data = wme->data;

	evtimer_del(&data->timer);
	screen_free(&data->screen);
	free(data);
}


// Source: window-clock.c
// Lines 178-185
