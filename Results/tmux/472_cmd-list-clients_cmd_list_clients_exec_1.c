cmd_list_clients_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args 		*args = cmd_get_args(self);
	struct cmd_find_state	*target = cmdq_get_target(item);
	struct client		*c;
	struct session		*s;
	struct format_tree	*ft;
	const char		*template;
	u_int			 idx;
	char			*line;

	if (args_has(args, 't'))
		s = target->s;
	else
		s = NULL;

	if ((template = args_get(args, 'F')) == NULL)
		template = LIST_CLIENTS_TEMPLATE;

	idx = 0;
	TAILQ_FOREACH(c, &clients, entry) {
		if (c->session == NULL || (s != NULL && s != c->session))
			continue;

		ft = format_create(cmdq_get_client(item), item, FORMAT_NONE, 0);
		format_add(ft, "line", "%u", idx);
		format_defaults(ft, c, NULL, NULL, NULL);

		line = format_expand(ft, template);
		cmdq_print(item, "%s", line);
		free(line);

		format_free(ft);

		idx++;
	}

	return (CMD_RETURN_NORMAL);
}


// Source: cmd-list-clients.c
// Lines 52-90
