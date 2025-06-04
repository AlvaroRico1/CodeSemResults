control_get_pane(struct client *c, struct window_pane *wp)
{
	struct control_state	*cs = c->control_state;
	struct control_pane	 cp = { .pane = wp->id };

	return (RB_FIND(control_panes, &cs->panes, &cp));
}


// Source: control.c
// Lines 235-241
