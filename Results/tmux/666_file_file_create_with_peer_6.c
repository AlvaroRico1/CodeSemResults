file_create_with_peer(struct tmuxpeer *peer, struct client_files *files,
    int stream, client_file_cb cb, void *cbdata)
{
	struct client_file	*cf;

	cf = xcalloc(1, sizeof *cf);
	cf->c = NULL;
	cf->references = 1;
	cf->stream = stream;

	cf->buffer = evbuffer_new();
	if (cf->buffer == NULL)
		fatalx("out of memory");

	cf->cb = cb;
	cf->data = cbdata;

	cf->peer = peer;
	cf->tree = files;
	RB_INSERT(client_files, files, cf);

	return (cf);
}


// Source: file.c
// Lines 71-93
