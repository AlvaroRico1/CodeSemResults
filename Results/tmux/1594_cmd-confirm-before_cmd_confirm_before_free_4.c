cmd_confirm_before_free(void *data)
{
	struct cmd_confirm_before_data	*cdata = data;

	cmd_list_free(cdata->cmdlist);
	free(cdata);
}


// Source: cmd-confirm-before.c
// Lines 136-142
