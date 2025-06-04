window_copy_free(struct window_mode_entry *wme)
{
	struct window_copy_mode_data	*data = wme->data;

	evtimer_del(&data->dragtimer);

	free(data->searchmark);
	free(data->searchstr);
	free(data->jumpchar);

	screen_free(data->backing);
	free(data->backing);

	screen_free(&data->screen);
	free(data);
}


// Source: window-copy.c
// Lines 485-500
