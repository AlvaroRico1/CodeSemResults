session_attach(struct session *s, struct window *w, int idx, char **cause)
{
	struct winlink	*wl;

	if ((wl = winlink_add(&s->windows, idx)) == NULL) {
		xasprintf(cause, "index in use: %d", idx);
		return (NULL);
	}
	wl->session = s;
	winlink_set_window(wl, w);
	notify_session_window("window-linked", s, w);

	session_group_synchronize_from(s);
	return (wl);
}


// Source: session.c
// Lines 336-350
