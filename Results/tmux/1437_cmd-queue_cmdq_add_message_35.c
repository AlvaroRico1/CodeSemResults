cmdq_add_message(struct cmdq_item *item)
{
	struct client		*c = item->client;
	struct cmdq_state	*state = item->state;
	const char		*name, *key;
	char			*tmp;

	tmp = cmd_print(item->cmd);
	if (c != NULL) {
		name = c->name;
		if (c->session != NULL && state->event.key != KEYC_NONE) {
			key = key_string_lookup_key(state->event.key, 0);
			server_add_message("%s key %s: %s", name, key, tmp);
		} else
			server_add_message("%s command: %s", name, tmp);
	} else
		server_add_message("command: %s", tmp);
	free(tmp);
}


// Source: cmd-queue.c
// Lines 557-575
