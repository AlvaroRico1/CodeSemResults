cmd_load_buffer_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args			*args = cmd_get_args(self);
	struct client			*tc = cmdq_get_target_client(item);
	struct cmd_load_buffer_data	*cdata;
	const char			*bufname = args_get(args, 'b');
	char				*path;

	cdata = xcalloc(1, sizeof *cdata);
	cdata->item = item;
	if (bufname != NULL)
		cdata->name = xstrdup(bufname);
	if (args_has(args, 'w') && tc != NULL) {
		cdata->client = tc;
		cdata->client->references++;
	}

	path = format_single_from_target(item, args_string(args, 0));
	file_read(cmdq_get_client(item), path, cmd_load_buffer_done, cdata);
	free(path);

	return (CMD_RETURN_WAIT);
}


// Source: cmd-load-buffer.c
// Lines 91-113
