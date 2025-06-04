cmd_detach_client_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args		*args = cmd_get_args(self);
	struct cmd_find_state	*source = cmdq_get_source(item);
	struct client		*tc = cmdq_get_target_client(item), *loop;
	struct session		*s;
	enum msgtype		 msgtype;
	const char		*cmd = args_get(args, 'E');

	if (cmd_get_entry(self) == &cmd_suspend_client_entry) {
		server_client_suspend(tc);
		return (CMD_RETURN_NORMAL);
	}

	if (args_has(args, 'P'))
		msgtype = MSG_DETACHKILL;
	else
		msgtype = MSG_DETACH;

	if (args_has(args, 's')) {
		s = source->s;
		if (s == NULL)
			return (CMD_RETURN_NORMAL);
		TAILQ_FOREACH(loop, &clients, entry) {
			if (loop->session == s) {
				if (cmd != NULL)
					server_client_exec(loop, cmd);
				else
					server_client_detach(loop, msgtype);
			}
		}
		return (CMD_RETURN_STOP);
	}

	if (args_has(args, 'a')) {
		TAILQ_FOREACH(loop, &clients, entry) {
			if (loop->session != NULL && loop != tc) {
				if (cmd != NULL)
					server_client_exec(loop, cmd);
				else
					server_client_detach(loop, msgtype);
			}
		}
		return (CMD_RETURN_NORMAL);
	}

	if (cmd != NULL)
		server_client_exec(tc, cmd);
	else
		server_client_detach(tc, msgtype);
	return (CMD_RETURN_STOP);
}


// Source: cmd-detach-client.c
// Lines 58-109
