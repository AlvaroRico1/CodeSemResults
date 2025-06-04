control_write_data(struct client *c, struct evbuffer *message)
{
	struct control_state	*cs = c->control_state;

	log_debug("%s: %s: %.*s", __func__, c->name,
	    (int)EVBUFFER_LENGTH(message), EVBUFFER_DATA(message));

	evbuffer_add(message, "\n", 1);
	bufferevent_write_buffer(cs->write_event, message);
	evbuffer_free(message);
}


// Source: control.c
// Lines 643-653
