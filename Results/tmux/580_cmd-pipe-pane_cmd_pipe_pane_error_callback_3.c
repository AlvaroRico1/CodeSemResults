cmd_pipe_pane_error_callback(__unused struct bufferevent *bufev,
    __unused short what, void *data)
{
	struct window_pane	*wp = data;

	log_debug("%%%u pipe error", wp->id);

	bufferevent_free(wp->pipe_event);
	close(wp->pipe_fd);
	wp->pipe_fd = -1;

	if (window_pane_destroy_ready(wp))
		server_destroy_pane(wp, 1);
}


// Source: cmd-pipe-pane.c
// Lines 211-224
