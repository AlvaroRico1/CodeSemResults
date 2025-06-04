control_notify_window_layout_changed(struct window *w)
{
	struct client	*c;
	struct session	*s;
	struct winlink	*wl;
	const char	*template;
	char		*cp;

	template = "%layout-change #{window_id} #{window_layout} "
	    "#{window_visible_layout} #{window_raw_flags}";

	TAILQ_FOREACH(c, &clients, entry) {
		if (!CONTROL_SHOULD_NOTIFY_CLIENT(c) || c->session == NULL)
			continue;
		s = c->session;

		if (winlink_find_by_window_id(&s->windows, w->id) == NULL)
			continue;

		/*
		 * When the last pane in a window is closed it won't have a
		 * layout root and we don't need to inform the client about the
		 * layout change because the whole window will go away soon.
		 */
		if (w->layout_root == NULL)
			continue;

		wl = winlink_find_by_window(&s->windows, w);
		if (wl != NULL) {
			cp = format_single(NULL, template, c, NULL, wl, NULL);
			control_write(c, "%s", cp);
			free(cp);
		}
	}
}


// Source: control-notify.c
// Lines 43-77
