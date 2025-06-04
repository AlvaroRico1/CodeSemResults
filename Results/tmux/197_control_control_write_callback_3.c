control_write_callback(__unused struct bufferevent *bufev, void *data)
{
	struct client		*c = data;
	struct control_state	*cs = c->control_state;
	struct control_pane	*cp, *cp1;
	struct evbuffer		*evb = cs->write_event->output;
	size_t			 space, limit;

	control_flush_all_blocks(c);

	while (EVBUFFER_LENGTH(evb) < CONTROL_BUFFER_HIGH) {
		if (cs->pending_count == 0)
			break;
		space = CONTROL_BUFFER_HIGH - EVBUFFER_LENGTH(evb);
		log_debug("%s: %s: %zu bytes available, %u panes", __func__,
		    c->name, space, cs->pending_count);

		limit = (space / cs->pending_count / 3); /* 3 bytes for \xxx */
		if (limit < CONTROL_WRITE_MINIMUM)
			limit = CONTROL_WRITE_MINIMUM;

		TAILQ_FOREACH_SAFE(cp, &cs->pending_list, pending_entry, cp1) {
			if (EVBUFFER_LENGTH(evb) >= CONTROL_BUFFER_HIGH)
				break;
			if (control_write_pending(c, cp, limit))
				continue;
			TAILQ_REMOVE(&cs->pending_list, cp, pending_entry);
			cp->pending_flag = 0;
			cs->pending_count--;
		}
	}
	if (EVBUFFER_LENGTH(evb) == 0)
		bufferevent_disable(cs->write_event, EV_WRITE);
}


// Source: control.c
// Lines 722-755
