cmd_find_best_client(struct session *s)
{
	struct client	*c_loop, *c;

	if (s->attached == 0)
		s = NULL;

	c = NULL;
	TAILQ_FOREACH(c_loop, &clients, entry) {
		if (c_loop->session == NULL)
			continue;
		if (s != NULL && c_loop->session != s)
			continue;
		if (cmd_find_client_better(c_loop, c))
			c = c_loop;
	}
	return (c);
}


// Source: cmd-find.c
// Lines 113-130
