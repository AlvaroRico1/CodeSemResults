file_create_with_client(struct client *c, int stream, client_file_cb cb,
    void *cbdata)
{
	struct client_file	*cf;

	if (c != NULL && (c->flags & CLIENT_ATTACHED))
		c = NULL;

	cf = xcalloc(1, sizeof *cf);
	cf->c = c;
	cf->references = 1;
	cf->stream = stream;

	cf->buffer = evbuffer_new();
	if (cf->buffer == NULL)
		fatalx("out of memory");

	cf->cb = cb;
	cf->data = cbdata;

	if (cf->c != NULL) {
		cf->peer = cf->c->peer;
		cf->tree = &cf->c->files;
		RB_INSERT(client_files, &cf->c->files, cf);
		cf->c->references++;
	}

	return (cf);
}


// Source: file.c
// Lines 97-125
