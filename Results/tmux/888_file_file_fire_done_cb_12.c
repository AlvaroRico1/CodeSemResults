file_fire_done_cb(__unused int fd, __unused short events, void *arg)
{
	struct client_file	*cf = arg;
	struct client		*c = cf->c;

	if (cf->cb != NULL && (c == NULL || (~c->flags & CLIENT_DEAD)))
		cf->cb(c, cf->path, cf->error, 1, cf->buffer, cf->data);
	file_free(cf);
}


// Source: file.c
// Lines 147-155
