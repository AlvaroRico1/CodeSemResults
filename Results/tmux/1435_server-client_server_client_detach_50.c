server_client_detach(struct client *c, enum msgtype msgtype)
{
	struct session	*s = c->session;

	if (s == NULL || (c->flags & CLIENT_UNATTACHEDFLAGS))
		return;

	c->flags |= CLIENT_EXIT;

	c->exit_type = CLIENT_EXIT_DETACH;
	c->exit_msgtype = msgtype;
	c->exit_session = xstrdup(s->name);
}


// Source: server-client.c
// Lines 512-524
