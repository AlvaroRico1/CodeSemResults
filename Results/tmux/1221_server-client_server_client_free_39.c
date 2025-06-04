server_client_free(__unused int fd, __unused short events, void *arg)
{
	struct client	*c = arg;

	log_debug("free client %p (%d references)", c, c->references);

	cmdq_free(c->queue);

	if (c->references == 0) {
		free((void *)c->name);
		free(c);
	}
}


// Source: server-client.c
// Lines 482-494
