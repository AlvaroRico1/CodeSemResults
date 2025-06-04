window_buffer_menu(void *modedata, struct client *c, key_code key)
{
	struct window_buffer_modedata	*data = modedata;
	struct window_pane		*wp = data->wp;
	struct window_mode_entry	*wme;

	wme = TAILQ_FIRST(&wp->modes);
	if (wme == NULL || wme->data != modedata)
		return;
	window_buffer_key(wme, c, NULL, NULL, key, NULL);
}


// Source: window-buffer.c
// Lines 279-289
