cmd_copy(struct cmd *cmd, int argc, char **argv)
{
	struct cmd	*new_cmd;

	new_cmd = xcalloc(1, sizeof *new_cmd);
	new_cmd->entry = cmd->entry;
	new_cmd->args = args_copy(cmd->args, argc, argv);

	if (cmd->file != NULL)
		new_cmd->file = xstrdup(cmd->file);
	new_cmd->line = cmd->line;

	return (new_cmd);
}


// Source: cmd.c
// Lines 548-561
