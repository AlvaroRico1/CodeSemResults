session_set_current(struct session *s, struct winlink *wl)
{
	struct winlink	*old = s->curw;

	if (wl == NULL)
		return (-1);
	if (wl == s->curw)
		return (1);

	winlink_stack_remove(&s->lastw, wl);
	winlink_stack_push(&s->lastw, s->curw);
	s->curw = wl;
	if (options_get_number(global_options, "focus-events")) {
		window_update_focus(old->window);
		window_update_focus(wl->window);
	}
	winlink_clear_flags(wl);
	window_update_activity(wl->window);
	tty_update_window_offset(wl->window);
	notify_session("session-window-changed", s);
	return (0);
}


// Source: session.c
// Lines 491-512
