alerts_check_silence(struct window *w)
{
	struct winlink	*wl;
	struct session	*s;

	if (~w->flags & WINDOW_SILENCE)
		return (0);
	if (options_get_number(w->options, "monitor-silence") == 0)
		return (0);

	TAILQ_FOREACH(wl, &w->winlinks, wentry)
		wl->session->flags &= ~SESSION_ALERTED;

	TAILQ_FOREACH(wl, &w->winlinks, wentry) {
		if (wl->flags & WINLINK_SILENCE)
			continue;
		s = wl->session;
		if (s->curw != wl || s->attached == 0) {
			wl->flags |= WINLINK_SILENCE;
			server_status_session(s);
		}
		if (!alerts_action_applies(wl, "silence-action"))
			continue;
		notify_winlink("alert-silence", wl);

		if (s->flags & SESSION_ALERTED)
			continue;
		s->flags |= SESSION_ALERTED;

		alerts_set_message(wl, "Silence", "visual-silence");
	}

	return (WINDOW_SILENCE);
}


// Source: alerts.c
// Lines 257-290
