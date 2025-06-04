cmd_list_panes_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args		*args = cmd_get_args(self);
	struct cmd_find_state	*target = cmdq_get_target(item);
	struct session		*s = target->s;
	struct winlink		*wl = target->wl;

	if (args_has(args, 'a'))
		cmd_list_panes_server(self, item);
	else if (args_has(args, 's'))
		cmd_list_panes_session(self, s, item, 1);
	else
		cmd_list_panes_window(self, s, wl, item, 0);

	return (CMD_RETURN_NORMAL);
}


// Source: cmd-list-panes.c
// Lines 51-66
