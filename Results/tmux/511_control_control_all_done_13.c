control_all_done(struct client *c)
{
	struct control_state	*cs = c->control_state;

	if (!TAILQ_EMPTY(&cs->all_blocks))
		return (0);
	return (EVBUFFER_LENGTH(cs->write_event->output) == 0);
}


// Source: control.c
// Lines 579-586
