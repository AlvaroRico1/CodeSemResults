server_client_dispatch_command(struct client *c, struct imsg *imsg)
{
	struct msg_command	  data;
	char			 *buf;
	size_t			  len;
	int			  argc;
	char			**argv, *cause;
	struct cmd_parse_result	 *pr;
	struct args_value	 *values;

	if (c->flags & CLIENT_EXIT)
		return;

	if (imsg->hdr.len - IMSG_HEADER_SIZE < sizeof data)
		fatalx("bad MSG_COMMAND size");
	memcpy(&data, imsg->data, sizeof data);

	buf = (char *)imsg->data + sizeof data;
	len = imsg->hdr.len  - IMSG_HEADER_SIZE - sizeof data;
	if (len > 0 && buf[len - 1] != '\0')
		fatalx("bad MSG_COMMAND string");

	argc = data.argc;
	if (cmd_unpack_argv(buf, len, argc, &argv) != 0) {
		cause = xstrdup("command too long");
		goto error;
	}

	if (argc == 0) {
		argc = 1;
		argv = xcalloc(1, sizeof *argv);
		*argv = xstrdup("new-session");
	}

	values = args_from_vector(argc, argv);
	pr = cmd_parse_from_arguments(values, argc, NULL);
	switch (pr->status) {
	case CMD_PARSE_ERROR:
		cause = pr->error;
		goto error;
	case CMD_PARSE_SUCCESS:
		break;
	}
	args_free_values(values, argc);
	free(values);
	cmd_free_argv(argc, argv);

	cmdq_append(c, cmdq_get_command(pr->cmdlist, NULL));
	cmdq_append(c, cmdq_get_callback(server_client_command_done, NULL));

	cmd_list_free(pr->cmdlist);
	return;

error:
	cmd_free_argv(argc, argv);

	cmdq_append(c, cmdq_get_error(cause));
	free(cause);

	c->flags |= CLIENT_EXIT;
}


// Source: server-client.c
// Lines 2165-2225
