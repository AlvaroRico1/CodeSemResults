server_client_set_flags(struct client *c, const char *flags)
{
	char	*s, *copy, *next;
	uint64_t flag;
	int	 not;

	s = copy = xstrdup(flags);
	while ((next = strsep(&s, ",")) != NULL) {
		not = (*next == '!');
		if (not)
			next++;

		if (c->flags & CLIENT_CONTROL)
			flag = server_client_control_flags(c, next);
		else
			flag = 0;
		if (strcmp(next, "read-only") == 0)
			flag = CLIENT_READONLY;
		else if (strcmp(next, "ignore-size") == 0)
			flag = CLIENT_IGNORESIZE;
		else if (strcmp(next, "active-pane") == 0)
			flag = CLIENT_ACTIVEPANE;
		if (flag == 0)
			continue;

		log_debug("client %s set flag %s", c->name, next);
		if (not)
			c->flags &= ~flag;
		else
			c->flags |= flag;
		if (flag == CLIENT_CONTROL_NOOUTPUT)
			control_reset_offsets(c);
	}
	free(copy);
	proc_send(c->peer, MSG_FLAGS, -1, &c->flags, sizeof c->flags);
}


// Source: server-client.c
// Lines 2424-2459
