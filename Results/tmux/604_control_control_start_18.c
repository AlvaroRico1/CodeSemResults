control_start(struct client *c)
{
	struct control_state	*cs;

	if (c->flags & CLIENT_CONTROLCONTROL) {
		close(c->out_fd);
		c->out_fd = -1;
	} else
		setblocking(c->out_fd, 0);
	setblocking(c->fd, 0);

	cs = c->control_state = xcalloc(1, sizeof *cs);
	RB_INIT(&cs->panes);
	TAILQ_INIT(&cs->pending_list);
	TAILQ_INIT(&cs->all_blocks);
	RB_INIT(&cs->subs);

	cs->read_event = bufferevent_new(c->fd, control_read_callback,
	    control_write_callback, control_error_callback, c);
	bufferevent_enable(cs->read_event, EV_READ);

	if (c->flags & CLIENT_CONTROLCONTROL)
		cs->write_event = cs->read_event;
	else {
		cs->write_event = bufferevent_new(c->out_fd, NULL,
		    control_write_callback, control_error_callback, c);
	}
	bufferevent_setwatermark(cs->write_event, EV_WRITE, CONTROL_BUFFER_LOW,
	    0);

	if (c->flags & CLIENT_CONTROLCONTROL) {
		bufferevent_write(cs->write_event, "\033P1000p", 7);
		bufferevent_enable(cs->write_event, EV_WRITE);
	}
}


// Source: control.c
// Lines 759-793
