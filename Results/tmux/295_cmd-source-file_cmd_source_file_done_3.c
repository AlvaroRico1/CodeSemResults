cmd_source_file_done(struct client *c, const char *path, int error,
    int closed, struct evbuffer *buffer, void *data)
{
	struct cmd_source_file_data	*cdata = data;
	struct cmdq_item		*item = cdata->item;
	void				*bdata = EVBUFFER_DATA(buffer);
	size_t				 bsize = EVBUFFER_LENGTH(buffer);
	u_int				 n;
	struct cmdq_item		*new_item;

	if (!closed)
		return;

	if (error != 0)
		cmdq_error(item, "%s: %s", path, strerror(error));
	else if (bsize != 0) {
		if (load_cfg_from_buffer(bdata, bsize, path, c, cdata->after,
		    cdata->flags, &new_item) < 0)
			cdata->retval = CMD_RETURN_ERROR;
		else if (new_item != NULL)
			cdata->after = new_item;
	}

	n = ++cdata->current;
	if (n < cdata->nfiles)
		file_read(c, cdata->files[n], cmd_source_file_done, cdata);
	else {
		cmd_source_file_complete(c, cdata);
		cmdq_continue(item);
	}
}


// Source: cmd-source-file.c
// Lines 86-116
