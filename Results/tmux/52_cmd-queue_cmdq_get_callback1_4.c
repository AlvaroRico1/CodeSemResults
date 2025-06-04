cmdq_get_callback1(const char *name, cmdq_cb cb, void *data)
{
	struct cmdq_item	*item;

	item = xcalloc(1, sizeof *item);
	xasprintf(&item->name, "[%s/%p]", name, item);
	item->type = CMDQ_CALLBACK;

	item->group = 0;
	item->state = cmdq_new_state(NULL, NULL, 0);

	item->cb = cb;
	item->data = data;

	return (item);
}


// Source: cmd-queue.c
// Lines 658-673
