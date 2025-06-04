server_client_check_modes(struct client *c)
{
	struct window			*w = c->session->curw->window;
	struct window_pane		*wp;
	struct window_mode_entry	*wme;

	if (c->flags & (CLIENT_CONTROL|CLIENT_SUSPENDED))
		return;
	if (~c->flags & CLIENT_REDRAWSTATUS)
		return;
	TAILQ_FOREACH(wp, &w->panes, entry) {
		wme = TAILQ_FIRST(&wp->modes);
		if (wme != NULL && wme->mode->update != NULL)
			wme->mode->update(wme);
	}
}


// Source: server-client.c
// Lines 1881-1896
