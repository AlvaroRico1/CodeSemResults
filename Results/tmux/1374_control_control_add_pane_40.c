control_add_pane(struct client *c, struct window_pane *wp)
{
	struct control_state	*cs = c->control_state;
	struct control_pane	*cp;

	cp = control_get_pane(c, wp);
	if (cp != NULL)
		return (cp);

	cp = xcalloc(1, sizeof *cp);
	cp->pane = wp->id;
	RB_INSERT(control_panes, &cs->panes, cp);

	memcpy(&cp->offset, &wp->offset, sizeof cp->offset);
	memcpy(&cp->queued, &wp->offset, sizeof cp->queued);
	TAILQ_INIT(&cp->blocks);

	return (cp);
}


// Source: control.c
// Lines 245-263
