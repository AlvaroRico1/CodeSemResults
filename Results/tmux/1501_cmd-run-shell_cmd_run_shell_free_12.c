cmd_run_shell_free(void *data)
{
	struct cmd_run_shell_data	*cdata = data;

	evtimer_del(&cdata->timer);
	if (cdata->s != NULL)
		session_remove_ref(cdata->s, __func__);
	if (cdata->client != NULL)
		server_client_unref(cdata->client);
	if (cdata->state != NULL)
		args_make_commands_free(cdata->state);
	free(cdata->cwd);
	free(cdata->cmd);
	free(cdata);
}


// Source: cmd-run-shell.c
// Lines 270-284
