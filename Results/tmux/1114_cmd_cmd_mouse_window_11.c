cmd_mouse_window(struct mouse_event *m, struct session **sp)
{
	struct session	*s;
	struct window	*w;
	struct winlink	*wl;

	if (!m->valid)
		return (NULL);
	if (m->s == -1 || (s = session_find_by_id(m->s)) == NULL)
		return (NULL);
	if (m->w == -1)
		wl = s->curw;
	else {
		if ((w = window_find_by_id(m->w)) == NULL)
			return (NULL);
		wl = winlink_find_by_window(&s->windows, w);
	}
	if (sp != NULL)
		*sp = s;
	return (wl);
}


// Source: cmd.c
// Lines 781-801
