server_client_set_pane(struct client *c, struct window_pane *wp)
{
	struct session		*s = c->session;
	struct client_window	*cw;

	if (s == NULL)
		return;

	cw = server_client_add_client_window(c, s->curw->window->id);
	cw->pane = wp;
	log_debug("%s pane now %%%u", c->name, wp->id);
}


// Source: server-client.c
// Lines 2543-2554
