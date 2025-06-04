cmd_if_shell_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args			*args = cmd_get_args(self);
	struct cmd_find_state		*target = cmdq_get_target(item);
	struct cmd_if_shell_data	*cdata;
	struct cmdq_item		*new_item;
	char				*shellcmd;
	struct client			*tc = cmdq_get_target_client(item);
	struct session			*s = target->s;
	struct cmd_list			*cmdlist;
	u_int				 count = args_count(args);
	int				 wait = !args_has(args, 'b');

	shellcmd = format_single_from_target(item, args_string(args, 0));
	if (args_has(args, 'F')) {
		if (*shellcmd != '0' && *shellcmd != '\0')
			cmdlist = args_make_commands_now(self, item, 1, 0);
		else if (count == 3)
			cmdlist = args_make_commands_now(self, item, 2, 0);
		else {
			free(shellcmd);
			return (CMD_RETURN_NORMAL);
		}
		free(shellcmd);
		if (cmdlist == NULL)
			return (CMD_RETURN_ERROR);
		new_item = cmdq_get_command(cmdlist, cmdq_get_state(item));
		cmdq_insert_after(item, new_item);
		return (CMD_RETURN_NORMAL);
	}

	cdata = xcalloc(1, sizeof *cdata);

	cdata->cmd_if = args_make_commands_prepare(self, item, 1, NULL, wait,
	    0);
	if (count == 3) {
		cdata->cmd_else = args_make_commands_prepare(self, item, 2,
		    NULL, wait, 0);
	}

	if (wait) {
		cdata->client = cmdq_get_client(item);
		cdata->item = item;
	} else
		cdata->client = tc;
	if (cdata->client != NULL)
		cdata->client->references++;

	if (job_run(shellcmd, 0, NULL, NULL, s,
	    server_client_get_cwd(cmdq_get_client(item), s), NULL,
	    cmd_if_shell_callback, cmd_if_shell_free, cdata, 0, -1,
	    -1) == NULL) {
		cmdq_error(item, "failed to run command: %s", shellcmd);
		free(shellcmd);
		free(cdata);
		return (CMD_RETURN_ERROR);
	}
	free(shellcmd);

	if (!wait)
		return (CMD_RETURN_NORMAL);
	return (CMD_RETURN_WAIT);
}


// Source: cmd-if-shell.c
// Lines 73-135
