cmd_confirm_before_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args			*args = cmd_get_args(self);
	struct cmd_confirm_before_data	*cdata;
	struct client			*tc = cmdq_get_target_client(item);
	struct cmd_find_state		*target = cmdq_get_target(item);
	char				*new_prompt;
	const char			*prompt, *cmd;
	int				 wait = !args_has(args, 'b');

	cdata = xcalloc(1, sizeof *cdata);
	cdata->cmdlist = args_make_commands_now(self, item, 0, 0);
	if (cdata->cmdlist == NULL)
		return (CMD_RETURN_ERROR);

	if (wait)
		cdata->item = item;

	if ((prompt = args_get(args, 'p')) != NULL)
		xasprintf(&new_prompt, "%s ", prompt);
	else {
		cmd = cmd_get_entry(cmd_list_first(cdata->cmdlist))->name;
		xasprintf(&new_prompt, "Confirm '%s'? (y/n) ", cmd);
	}

	status_prompt_set(tc, target, new_prompt, NULL,
	    cmd_confirm_before_callback, cmd_confirm_before_free, cdata,
	    PROMPT_SINGLE, PROMPT_TYPE_COMMAND);
	free(new_prompt);

	if (!wait)
		return (CMD_RETURN_NORMAL);
	return (CMD_RETURN_WAIT);
}


// Source: cmd-confirm-before.c
// Lines 64-97
