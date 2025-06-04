server_client_update_latest(struct client *c)
{
	struct window	*w;

	if (c->session == NULL)
		return;
	w = c->session->curw->window;

	if (w->latest == c)
		return;
	w->latest = c;

	if (options_get_number(w->options, "window-size") == WINDOW_SIZE_LATEST)
		recalculate_size(w, 0);

	notify_client("client-active", c);
}


// Source: server-client.c
// Lines 1193-1209
