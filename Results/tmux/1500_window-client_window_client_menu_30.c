window_client_menu(void *modedata, struct client *c, key_code key)
{
	struct window_client_modedata	*data = modedata;
	struct window_pane		*wp = data->wp;
	struct window_mode_entry	*wme;

	wme = TAILQ_FIRST(&wp->modes);
	if (wme == NULL || wme->data != modedata)
		return;
	window_client_key(wme, c, NULL, NULL, key, NULL);
}


// Source: window-client.c
// Lines 255-265
