control_notify_window_renamed(struct window *w)
{
	struct client	*c;
	struct session	*cs;

	TAILQ_FOREACH(c, &clients, entry) {
		if (!CONTROL_SHOULD_NOTIFY_CLIENT(c) || c->session == NULL)
			continue;
		cs = c->session;

		if (winlink_find_by_window_id(&cs->windows, w->id) != NULL) {
			control_write(c, "%%window-renamed @%u %s", w->id,
			    w->name);
		} else {
			control_write(c, "%%unlinked-window-renamed @%u %s",
			    w->id, w->name);
		}
	}
}


// Source: control-notify.c
// Lines 130-148
