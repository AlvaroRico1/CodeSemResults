status_line_size(struct client *c)
{
	struct session	*s = c->session;

	if (c->flags & (CLIENT_STATUSOFF|CLIENT_CONTROL))
		return (0);
	if (s == NULL)
		return (options_get_number(global_s_options, "status"));
	return (s->statuslines);
}


// Source: status.c
// Lines 255-264
