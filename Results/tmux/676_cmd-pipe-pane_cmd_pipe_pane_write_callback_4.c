cmd_pipe_pane_write_callback(__unused struct bufferevent *bufev, void *data)
{
	struct window_pane	*wp = data;

	log_debug("%%%u pipe empty", wp->id);

	if (window_pane_destroy_ready(wp))
		server_destroy_pane(wp, 1);
}


// Source: cmd-pipe-pane.c
// Lines 200-208
