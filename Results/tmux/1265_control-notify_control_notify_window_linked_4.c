control_notify_window_linked(__unused struct session *s, struct window *w)
{
	struct client	*c;
	struct session	*cs;

	TAILQ_FOREACH(c, &clients, entry) {
		if (!CONTROL_SHOULD_NOTIFY_CLIENT(c) || c->session == NULL)
			continue;
		cs = c->session;

		if (winlink_find_by_window_id(&cs->windows, w->id) != NULL)
			control_write(c, "%%window-add @%u", w->id);
		else
			control_write(c, "%%unlinked-window-add @%u", w->id);
	}
}


// Source: control-notify.c
// Lines 112-127
