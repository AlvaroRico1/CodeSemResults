cmd_display_panes_free(__unused struct client *c, void *data)
{
	struct cmd_display_panes_data	*cdata = data;

	if (cdata->item != NULL)
		cmdq_continue(cdata->item);
	args_make_commands_free(cdata->state);
	free(cdata);
}


// Source: cmd-display-panes.c
// Lines 213-221
