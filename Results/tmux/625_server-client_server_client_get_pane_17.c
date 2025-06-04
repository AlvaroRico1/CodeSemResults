server_client_get_pane(struct client *c)
{
	struct session		*s = c->session;
	struct client_window	*cw;

	if (s == NULL)
		return (NULL);

	if (~c->flags & CLIENT_ACTIVEPANE)
		return (s->curw->window->active);
	cw = server_client_get_client_window(c, s->curw->window->id);
	if (cw == NULL)
		return (s->curw->window->active);
	return (cw->pane);
}


// Source: server-client.c
// Lines 2525-2539
