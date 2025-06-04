cmd_save_buffer_done(__unused struct client *c, const char *path, int error,
    __unused int closed, __unused struct evbuffer *buffer, void *data)
{
	struct cmdq_item	*item = data;

	if (!closed)
		return;

	if (error != 0)
		cmdq_error(item, "%s: %s", path, strerror(error));
	cmdq_continue(item);
}


// Source: cmd-save-buffer.c
// Lines 59-70
