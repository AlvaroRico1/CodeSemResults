session_free(__unused int fd, __unused short events, void *arg)
{
	struct session	*s = arg;

	log_debug("session %s freed (%d references)", s->name, s->references);

	if (s->references == 0) {
		environ_free(s->environ);
		options_free(s->options);

		free(s->name);
		free(s);
	}
}


// Source: session.c
// Lines 184-197
