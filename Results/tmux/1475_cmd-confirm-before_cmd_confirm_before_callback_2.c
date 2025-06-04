cmd_confirm_before_callback(struct client *c, void *data, const char *s,
    __unused int done)
{
	struct cmd_confirm_before_data	*cdata = data;
	struct cmdq_item		*item = cdata->item, *new_item;
	int				 retcode = 1;

	if (c->flags & CLIENT_DEAD)
		goto out;

	if (s == NULL || *s == '\0')
		goto out;
	if (tolower((u_char)s[0]) != 'y' || s[1] != '\0')
		goto out;
	retcode = 0;

	if (item == NULL) {
		new_item = cmdq_get_command(cdata->cmdlist, NULL);
		cmdq_append(c, new_item);
	} else {
		new_item = cmdq_get_command(cdata->cmdlist,
		    cmdq_get_state(item));
		cmdq_insert_after(item, new_item);
	}

out:
        if (item != NULL) {
                if (cmdq_get_client(item) != NULL &&
                    cmdq_get_client(item)->session == NULL)
                        cmdq_get_client(item)->retval = retcode;
                cmdq_continue(item);
        }
	return (0);
}


// Source: cmd-confirm-before.c
// Lines 100-133
