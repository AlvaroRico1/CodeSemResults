control_write(struct client *c, const char *fmt, ...)
{
	struct control_state	*cs = c->control_state;
	struct control_block	*cb;
	va_list			 ap;

	va_start(ap, fmt);

	if (TAILQ_EMPTY(&cs->all_blocks)) {
		control_vwrite(c, fmt, ap);
		va_end(ap);
		return;
	}

	cb = xcalloc(1, sizeof *cb);
	xvasprintf(&cb->line, fmt, ap);
	TAILQ_INSERT_TAIL(&cs->all_blocks, cb, all_entry);
	cb->t = get_timer();

	log_debug("%s: %s: storing line: %s", __func__, c->name, cb->line);
	bufferevent_enable(cs->write_event, EV_WRITE);

	va_end(ap);
}


// Source: control.c
// Lines 406-429
