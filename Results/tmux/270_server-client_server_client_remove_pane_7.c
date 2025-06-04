server_client_remove_pane(struct window_pane *wp)
{
	struct client		*c;
	struct window		*w = wp->window;
	struct client_window	*cw;

	TAILQ_FOREACH(c, &clients, entry) {
		cw = server_client_get_client_window(c, w->id);
		if (cw != NULL && cw->pane == wp) {
			RB_REMOVE(client_windows, &c->windows, cw);
			free(cw);
		}
	}
}


// Source: server-client.c
// Lines 2558-2571
