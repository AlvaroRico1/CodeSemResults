cmd_parse(struct args_value *values, u_int count, const char *file, u_int line,
    char **cause)
{
	const struct cmd_entry	*entry;
	struct cmd		*cmd;
	struct args		*args;
	char			*error = NULL;

	if (count == 0 || values[0].type != ARGS_STRING) {
		xasprintf(cause, "no command");
		return (NULL);
	}
	entry = cmd_find(values[0].string, cause);
	if (entry == NULL)
		return (NULL);

	args = args_parse(&entry->args, values, count, &error);
	if (args == NULL && error == NULL) {
		xasprintf(cause, "usage: %s %s", entry->name, entry->usage);
		return (NULL);
	}
	if (args == NULL) {
		xasprintf(cause, "command %s: %s", entry->name, error);
		free(error);
		return (NULL);
	}

	cmd = xcalloc(1, sizeof *cmd);
	cmd->entry = entry;
	cmd->args = args;

	if (file != NULL)
		cmd->file = xstrdup(file);
	cmd->line = line;

	return (cmd);
}


// Source: cmd.c
// Lines 498-534
