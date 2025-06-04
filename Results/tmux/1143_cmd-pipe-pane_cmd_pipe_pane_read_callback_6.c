cmd_pipe_pane_read_callback(__unused struct bufferevent *bufev, void *data)
{
	struct window_pane	*wp = data;
	struct evbuffer		*evb = wp->pipe_event->input;
	size_t			 available;

	available = EVBUFFER_LENGTH(evb);
	log_debug("%%%u pipe read %zu", wp->id, available);

	bufferevent_write(wp->event, EVBUFFER_DATA(evb), available);
	evbuffer_drain(evb, available);

	if (window_pane_destroy_ready(wp))
		server_destroy_pane(wp, 1);
}


// Source: cmd-pipe-pane.c
// Lines 183-197
