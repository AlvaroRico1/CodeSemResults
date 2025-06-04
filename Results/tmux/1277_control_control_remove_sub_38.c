control_remove_sub(struct client *c, const char *name)
{
	struct control_state	*cs = c->control_state;
	struct control_sub	*csub, find;

	find.name = (char *)name;
	if ((csub = RB_FIND(control_subs, &cs->subs, &find)) != NULL)
		control_free_sub(cs, csub);
	if (RB_EMPTY(&cs->subs))
		evtimer_del(&cs->subs_timer);
}


// Source: control.c
// Lines 1097-1107
