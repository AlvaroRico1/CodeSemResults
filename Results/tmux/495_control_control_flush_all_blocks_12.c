control_flush_all_blocks(struct client *c)
{
	struct control_state	*cs = c->control_state;
	struct control_block	*cb, *cb1;

	TAILQ_FOREACH_SAFE(cb, &cs->all_blocks, all_entry, cb1) {
		if (cb->size != 0)
			break;
		log_debug("%s: %s: flushing line: %s", __func__, c->name,
		    cb->line);

		bufferevent_write(cs->write_event, cb->line, strlen(cb->line));
		bufferevent_write(cs->write_event, "\n", 1);
		control_free_block(cs, cb);
	}
}


// Source: control.c
// Lines 590-605
