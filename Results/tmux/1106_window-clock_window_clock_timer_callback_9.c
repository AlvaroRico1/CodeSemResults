window_clock_timer_callback(__unused int fd, __unused short events, void *arg)
{
	struct window_mode_entry	*wme = arg;
	struct window_pane		*wp = wme->wp;
	struct window_clock_mode_data	*data = wme->data;
	struct tm			 now, then;
	time_t				 t;
	struct timeval			 tv = { .tv_sec = 1 };

	evtimer_del(&data->timer);
	evtimer_add(&data->timer, &tv);

	if (TAILQ_FIRST(&wp->modes) != wme)
		return;

	t = time(NULL);
	gmtime_r(&t, &now);
	gmtime_r(&data->tim, &then);
	if (now.tm_min == then.tm_min)
		return;
	data->tim = t;

	window_clock_draw_screen(wme);
	wp->flags |= PANE_REDRAW;
}


// Source: window-clock.c
// Lines 127-151
