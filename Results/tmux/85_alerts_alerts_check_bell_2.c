alerts_check_bell(struct window *w)
{
	struct winlink	*wl;
	struct session	*s;

	if (~w->flags & WINDOW_BELL)
		return (0);
	if (!options_get_number(w->options, "monitor-bell"))
		return (0);

	TAILQ_FOREACH(wl, &w->winlinks, wentry)
		wl->session->flags &= ~SESSION_ALERTED;

	TAILQ_FOREACH(wl, &w->winlinks, wentry) {
		/*
		 * Bells are allowed even if there is an existing bell (so do
		 * not check WINLINK_BELL).
		 */
		s = wl->session;
		if (s->curw != wl || s->attached == 0) {
			wl->flags |= WINLINK_BELL;
			server_status_session(s);
		}
		if (!alerts_action_applies(wl, "bell-action"))
			continue;
		notify_winlink("alert-bell", wl);

		if (s->flags & SESSION_ALERTED)
			continue;
		s->flags |= SESSION_ALERTED;

		alerts_set_message(wl, "Bell", "visual-bell");
	}

	return (WINDOW_BELL);
}


// Source: alerts.c
// Lines 183-218
