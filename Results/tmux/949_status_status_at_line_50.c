status_at_line(struct client *c)
{
	struct session	*s = c->session;

	if (c->flags & (CLIENT_STATUSOFF|CLIENT_CONTROL))
		return (-1);
	if (s->statusat != 1)
		return (s->statusat);
	return (c->tty.sy - status_line_size(c));
}


// Source: status.c
// Lines 242-251
