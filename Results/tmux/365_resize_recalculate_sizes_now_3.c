recalculate_sizes_now(int now)
{
	struct session	*s;
	struct client	*c;
	struct window	*w;

	/*
	 * Clear attached count and update saved status line information for
	 * each session.
	 */
	RB_FOREACH(s, sessions, &sessions) {
		s->attached = 0;
		status_update_cache(s);
	}

	/*
	 * Increment attached count and check the status line size for each
	 * client.
	 */
	TAILQ_FOREACH(c, &clients, entry) {
		s = c->session;
		if (s != NULL && !(c->flags & CLIENT_UNATTACHEDFLAGS))
			s->attached++;
		if (ignore_client_size(c))
			continue;
		if (c->tty.sy <= s->statuslines || (c->flags & CLIENT_CONTROL))
			c->flags |= CLIENT_STATUSOFF;
		else
			c->flags &= ~CLIENT_STATUSOFF;
	}

	/* Walk each window and adjust the size. */
	RB_FOREACH(w, windows, &windows)
		recalculate_size(w, now);
}


// Source: resize.c
// Lines 430-464
