server_kill_pane(struct window_pane *wp)
{
	struct window	*w = wp->window;

	if (window_count_panes(w) == 1) {
		server_kill_window(w, 1);
		recalculate_sizes();
	} else {
		server_unzoom_window(w);
		server_client_remove_pane(wp);
		layout_close_pane(wp);
		window_remove_pane(w, wp);
		server_redraw_window(w);
	}
}


// Source: server-fn.c
// Lines 179-193
