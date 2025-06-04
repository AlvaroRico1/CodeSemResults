control_stop(struct client *c)
{
	struct control_state	*cs = c->control_state;
	struct control_block	*cb, *cb1;
	struct control_sub	*csub, *csub1;

	if (~c->flags & CLIENT_CONTROLCONTROL)
		bufferevent_free(cs->write_event);
	bufferevent_free(cs->read_event);

	RB_FOREACH_SAFE(csub, control_subs, &cs->subs, csub1)
		control_free_sub(cs, csub);
	if (evtimer_initialized(&cs->subs_timer))
		evtimer_del(&cs->subs_timer);

	TAILQ_FOREACH_SAFE(cb, &cs->all_blocks, all_entry, cb1)
		control_free_block(cs, cb);
	control_reset_offsets(c);

	free(cs);
}


// Source: control.c
// Lines 809-829
