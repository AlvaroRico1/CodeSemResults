control_vwrite(struct client *c, const char *fmt, va_list ap)
{
	struct control_state	*cs = c->control_state;
	char			*s;

	xvasprintf(&s, fmt, ap);
	log_debug("%s: %s: writing line: %s", __func__, c->name, s);

	bufferevent_write(cs->write_event, s, strlen(s));
	bufferevent_write(cs->write_event, "\n", 1);

	bufferevent_enable(cs->write_event, EV_WRITE);
	free(s);
}


// Source: control.c
// Lines 389-402
