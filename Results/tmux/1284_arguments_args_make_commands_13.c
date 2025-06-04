args_make_commands(struct args_command_state *state, int argc, char **argv,
    char **error)
{
	struct cmd_parse_result	*pr;
	char			*cmd, *new_cmd;
	int			 i;

	if (state->cmdlist != NULL) {
		if (argc == 0)
			return (state->cmdlist);
		return (cmd_list_copy(state->cmdlist, argc, argv));
	}

	cmd = xstrdup(state->cmd);
	for (i = 0; i < argc; i++) {
		new_cmd = cmd_template_replace(cmd, argv[i], i + 1);
		log_debug("%s: %%%u %s: %s", __func__, i + 1, argv[i], new_cmd);
		free(cmd);
		cmd = new_cmd;
	}
	log_debug("%s: %s", __func__, cmd);

	pr = cmd_parse_from_string(cmd, &state->pi);
	free(cmd);
	switch (pr->status) {
	case CMD_PARSE_ERROR:
		*error = pr->error;
		return (NULL);
	case CMD_PARSE_SUCCESS:
		return (pr->cmdlist);
	}
}


// Source: arguments.c
// Lines 725-756
