control_reset_offsets(struct client *c)
{
	struct control_state	*cs = c->control_state;
	struct control_pane	*cp, *cp1;

	RB_FOREACH_SAFE(cp, control_panes, &cs->panes, cp1) {
		RB_REMOVE(control_panes, &cs->panes, cp);
		free(cp);
	}

	TAILQ_INIT(&cs->pending_list);
	cs->pending_count = 0;
}


// Source: control.c
// Lines 295-307
