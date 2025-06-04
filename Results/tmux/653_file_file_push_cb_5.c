file_push_cb(__unused int fd, __unused short events, void *arg)
{
	struct client_file	*cf = arg;

	if (cf->c == NULL || ~cf->c->flags & CLIENT_DEAD)
		file_push(cf);
	file_free(cf);
}


// Source: file.c
// Lines 431-438
