popup_free_cb(struct client *c, void *data)
{
	struct popup_data	*pd = data;
	struct cmdq_item	*item = pd->item;

	if (pd->md != NULL)
		menu_free_cb(c, pd->md);

	if (pd->cb != NULL)
		pd->cb(pd->status, pd->arg);

	if (item != NULL) {
		if (cmdq_get_client(item) != NULL &&
		    cmdq_get_client(item)->session == NULL)
			cmdq_get_client(item)->retval = pd->status;
		cmdq_continue(item);
	}
	server_client_unref(pd->c);

	if (pd->job != NULL)
		job_free(pd->job);
	input_free(pd->ictx);

	screen_free(&pd->s);
	colour_palette_free(&pd->palette);

	free(pd->title);
	free(pd);
}


// Source: popup.c
// Lines 265-293
