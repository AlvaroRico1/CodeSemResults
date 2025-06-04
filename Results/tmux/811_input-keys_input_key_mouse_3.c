input_key_mouse(struct window_pane *wp, struct mouse_event *m)
{
	struct screen	*s = wp->screen;
	u_int		 x, y;
	const char	*buf;
	size_t		 len;

	/* Ignore events if no mouse mode or the pane is not visible. */
	if (m->ignore || (s->mode & ALL_MOUSE_MODES) == 0)
		return;
	if (cmd_mouse_at(wp, m, &x, &y, 0) != 0)
		return;
	if (!window_pane_visible(wp))
		return;
	if (!input_key_get_mouse(s, m, x, y, &buf, &len))
		return;
	log_debug("writing mouse %.*s to %%%u", (int)len, buf, wp->id);
	input_key_write(__func__, wp->event, buf, len);
}


// Source: input-keys.c
// Lines 626-644
