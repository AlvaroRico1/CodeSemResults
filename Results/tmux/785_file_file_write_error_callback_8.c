file_write_error_callback(__unused struct bufferevent *bev, __unused short what,
    void *arg)
{
	struct client_file	*cf = arg;

	log_debug("write error file %d", cf->stream);

	bufferevent_free(cf->event);
	cf->event = NULL;

	close(cf->fd);
	cf->fd = -1;

	if (cf->cb != NULL)
		cf->cb(NULL, NULL, 0, -1, NULL, cf->data);
}


// Source: file.c
// Lines 499-514
