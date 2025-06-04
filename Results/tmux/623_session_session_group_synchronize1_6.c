session_group_synchronize1(struct session *target, struct session *s)
{
	struct winlinks		 old_windows, *ww;
	struct winlink_stack	 old_lastw;
	struct winlink		*wl, *wl2;

	/* Don't do anything if the session is empty (it'll be destroyed). */
	ww = &target->windows;
	if (RB_EMPTY(ww))
		return;

	/* If the current window has vanished, move to the next now. */
	if (s->curw != NULL &&
	    winlink_find_by_index(ww, s->curw->idx) == NULL &&
	    session_last(s) != 0 && session_previous(s, 0) != 0)
		session_next(s, 0);

	/* Save the old pointer and reset it. */
	memcpy(&old_windows, &s->windows, sizeof old_windows);
	RB_INIT(&s->windows);

	/* Link all the windows from the target. */
	RB_FOREACH(wl, winlinks, ww) {
		wl2 = winlink_add(&s->windows, wl->idx);
		wl2->session = s;
		winlink_set_window(wl2, wl->window);
		notify_session_window("window-linked", s, wl2->window);
		wl2->flags |= wl->flags & WINLINK_ALERTFLAGS;
	}

	/* Fix up the current window. */
	if (s->curw != NULL)
		s->curw = winlink_find_by_index(&s->windows, s->curw->idx);
	else
		s->curw = winlink_find_by_index(&s->windows, target->curw->idx);

	/* Fix up the last window stack. */
	memcpy(&old_lastw, &s->lastw, sizeof old_lastw);
	TAILQ_INIT(&s->lastw);
	TAILQ_FOREACH(wl, &old_lastw, sentry) {
		wl2 = winlink_find_by_index(&s->windows, wl->idx);
		if (wl2 != NULL)
			TAILQ_INSERT_TAIL(&s->lastw, wl2, sentry);
	}

	/* Then free the old winlinks list. */
	while (!RB_EMPTY(&old_windows)) {
		wl = RB_ROOT(&old_windows);
		wl2 = winlink_find_by_window_id(&s->windows, wl->window->id);
		if (wl2 == NULL)
			notify_session_window("window-unlinked", s, wl->window);
		winlink_remove(&old_windows, wl);
	}
}


// Source: session.c
// Lines 648-701
