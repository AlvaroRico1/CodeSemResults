input_key_get_mouse(struct screen *s, struct mouse_event *m, u_int x, u_int y,
    const char **rbuf, size_t *rlen)
{
	static char	 buf[40];
	size_t		 len;

	*rbuf = NULL;
	*rlen = 0;

	/* If this pane is not in button or all mode, discard motion events. */
	if (MOUSE_DRAG(m->b) && (s->mode & MOTION_MOUSE_MODES) == 0)
		return (0);
	if ((s->mode & ALL_MOUSE_MODES) == 0)
		return (0);

	/*
	 * If this event is a release event and not in all mode, discard it.
	 * In SGR mode we can tell absolutely because a release is normally
	 * shown by the last character. Without SGR, we check if the last
	 * buttons was also a release.
	 */
	if (m->sgr_type != ' ') {
		if (MOUSE_DRAG(m->sgr_b) &&
		    MOUSE_BUTTONS(m->sgr_b) == 3 &&
		    (~s->mode & MODE_MOUSE_ALL))
			return (0);
	} else {
		if (MOUSE_DRAG(m->b) &&
		    MOUSE_BUTTONS(m->b) == 3 &&
		    MOUSE_BUTTONS(m->lb) == 3 &&
		    (~s->mode & MODE_MOUSE_ALL))
			return (0);
	}

	/*
	 * Use the SGR (1006) extension only if the application requested it
	 * and the underlying terminal also sent the event in this format (this
	 * is because an old style mouse release event cannot be converted into
	 * the new SGR format, since the released button is unknown). Otherwise
	 * pretend that tmux doesn't speak this extension, and fall back to the
	 * UTF-8 (1005) extension if the application requested, or to the
	 * legacy format.
	 */
	if (m->sgr_type != ' ' && (s->mode & MODE_MOUSE_SGR)) {
		len = xsnprintf(buf, sizeof buf, "\033[<%u;%u;%u%c",
		    m->sgr_b, x + 1, y + 1, m->sgr_type);
	} else if (s->mode & MODE_MOUSE_UTF8) {
		if (m->b > 0x7ff - 32 || x > 0x7ff - 33 || y > 0x7ff - 33)
			return (0);
		len = xsnprintf(buf, sizeof buf, "\033[M");
		len += input_key_split2(m->b + 32, &buf[len]);
		len += input_key_split2(x + 33, &buf[len]);
		len += input_key_split2(y + 33, &buf[len]);
	} else {
		if (m->b > 223)
			return (0);
		len = xsnprintf(buf, sizeof buf, "\033[M");
		buf[len++] = m->b + 32;
		buf[len++] = x + 33;
		buf[len++] = y + 33;
	}

	*rbuf = buf;
	*rlen = len;
	return (1);
}


// Source: input-keys.c
// Lines 557-622
