control_notify_client_session_changed(struct client *cc)
{
	struct client	*c;
	struct session	*s;

	if (cc->session == NULL)
		return;
	s = cc->session;

	TAILQ_FOREACH(c, &clients, entry) {
		if (!CONTROL_SHOULD_NOTIFY_CLIENT(c) || c->session == NULL)
			continue;

		if (cc == c) {
			control_write(c, "%%session-changed $%u %s", s->id,
			    s->name);
		} else {
			control_write(c, "%%client-session-changed %s $%u %s",
			    cc->name, s->id, s->name);
		}
	}
}


// Source: control-notify.c
// Lines 151-172
