cmd_load_buffer_done(__unused struct client *c, const char *path, int error,
    int closed, struct evbuffer *buffer, void *data)
{
	struct cmd_load_buffer_data	*cdata = data;
	struct client			*tc = cdata->client;
	struct cmdq_item		*item = cdata->item;
	void				*bdata = EVBUFFER_DATA(buffer);
	size_t				 bsize = EVBUFFER_LENGTH(buffer);
	void				*copy;
	char				*cause;

	if (!closed)
		return;

	if (error != 0)
		cmdq_error(item, "%s: %s", path, strerror(error));
	else if (bsize != 0) {
		copy = xmalloc(bsize);
		memcpy(copy, bdata, bsize);
		if (paste_set(copy, bsize, cdata->name, &cause) != 0) {
			cmdq_error(item, "%s", cause);
			free(cause);
			free(copy);
		} else if (tc != NULL &&
		    tc->session != NULL &&
		    (~tc->flags & CLIENT_DEAD))
			tty_set_selection(&tc->tty, copy, bsize);
		if (tc != NULL)
			server_client_unref(tc);
	}
	cmdq_continue(item);

	free(cdata->name);
	free(cdata);
}


// Source: cmd-load-buffer.c
// Lines 54-88
