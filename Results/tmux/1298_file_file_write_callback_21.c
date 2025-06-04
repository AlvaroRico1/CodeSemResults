file_write_callback(__unused struct bufferevent *bev, void *arg)
{
	struct client_file	*cf = arg;

	log_debug("write check file %d", cf->stream);

	if (cf->cb != NULL)
		cf->cb(NULL, NULL, 0, -1, NULL, cf->data);

	if (cf->closed && EVBUFFER_LENGTH(cf->event->output) == 0) {
		bufferevent_free(cf->event);
		close(cf->fd);
		RB_REMOVE(client_files, cf->tree, cf);
		file_free(cf);
	}
}


// Source: file.c
// Lines 518-533
