cmd_command_prompt_free(void *data)
{
	struct cmd_command_prompt_cdata	*cdata = data;
	u_int				 i;

	for (i = 0; i < cdata->count; i++) {
		free(cdata->prompts[i].prompt);
		free(cdata->prompts[i].input);
	}
	free(cdata->prompts);
	cmd_free_argv(cdata->argc, cdata->argv);
	args_make_commands_free(cdata->state);
	free(cdata);
}


// Source: cmd-command-prompt.c
// Lines 225-238
