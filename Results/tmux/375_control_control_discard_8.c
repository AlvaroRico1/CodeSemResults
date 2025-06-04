control_discard(struct client *c)
{
	struct control_state	*cs = c->control_state;
	struct control_pane	*cp;

	RB_FOREACH(cp, control_panes, &cs->panes)
		control_discard_pane(c, cp);
	bufferevent_disable(cs->read_event, EV_READ);
}


// Source: control.c
// Lines 797-805
