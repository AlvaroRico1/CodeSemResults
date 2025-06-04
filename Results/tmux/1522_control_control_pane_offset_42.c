control_pane_offset(struct client *c, struct window_pane *wp, int *off)
{
	struct control_state	*cs = c->control_state;
	struct control_pane	*cp;

	if (c->flags & CLIENT_CONTROL_NOOUTPUT) {
		*off = 0;
		return (NULL);
	}

	cp = control_get_pane(c, wp);
	if (cp == NULL || (cp->flags & CONTROL_PANE_PAUSED)) {
		*off = 0;
		return (NULL);
	}
	if (cp->flags & CONTROL_PANE_OFF) {
		*off = 1;
		return (NULL);
	}
	*off = (EVBUFFER_LENGTH(cs->write_event->output) >= CONTROL_BUFFER_LOW);
	return (&cp->offset);
}


// Source: control.c
// Lines 311-332
