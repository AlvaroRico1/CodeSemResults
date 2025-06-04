cmd_if_shell_free(void *data)
{
	struct cmd_if_shell_data	*cdata = data;

	if (cdata->client != NULL)
		server_client_unref(cdata->client);

	if (cdata->cmd_else != NULL)
		args_make_commands_free(cdata->cmd_else);
	args_make_commands_free(cdata->cmd_if);

	free(cdata);
}


// Source: cmd-if-shell.c
// Lines 178-190
