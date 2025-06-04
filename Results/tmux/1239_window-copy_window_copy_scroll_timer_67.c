window_copy_scroll_timer(__unused int fd, __unused short events, void *arg)
{
	struct window_mode_entry	*wme = arg;
	struct window_pane		*wp = wme->wp;
	struct window_copy_mode_data	*data = wme->data;
	struct timeval			 tv = {
		.tv_usec = WINDOW_COPY_DRAG_REPEAT_TIME
	};

	evtimer_del(&data->dragtimer);

	if (TAILQ_FIRST(&wp->modes) != wme)
		return;

	if (data->cy == 0) {
		evtimer_add(&data->dragtimer, &tv);
		window_copy_cursor_up(wme, 1);
	} else if (data->cy == screen_size_y(&data->screen) - 1) {
		evtimer_add(&data->dragtimer, &tv);
		window_copy_cursor_down(wme, 1);
	}
}


// Source: window-copy.c
// Lines 303-324
