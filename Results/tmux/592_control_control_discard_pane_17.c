control_discard_pane(struct client *c, struct control_pane *cp)
{
	struct control_state	*cs = c->control_state;
	struct control_block	*cb, *cb1;

	TAILQ_FOREACH_SAFE(cb, &cp->blocks, entry, cb1) {
		TAILQ_REMOVE(&cp->blocks, cb, entry);
		control_free_block(cs, cb);
	}
}


// Source: control.c
// Lines 267-276
