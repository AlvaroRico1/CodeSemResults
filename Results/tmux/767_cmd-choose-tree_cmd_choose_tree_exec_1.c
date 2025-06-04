cmd_choose_tree_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args			*args = cmd_get_args(self);
	struct cmd_find_state		*target = cmdq_get_target(item);
	struct window_pane		*wp = target->wp;
	const struct window_mode	*mode;

	if (cmd_get_entry(self) == &cmd_choose_buffer_entry) {
		if (paste_get_top(NULL) == NULL)
			return (CMD_RETURN_NORMAL);
		mode = &window_buffer_mode;
	} else if (cmd_get_entry(self) == &cmd_choose_client_entry) {
		if (server_client_how_many() == 0)
			return (CMD_RETURN_NORMAL);
		mode = &window_client_mode;
	} else if (cmd_get_entry(self) == &cmd_customize_mode_entry)
		mode = &window_customize_mode;
	else
		mode = &window_tree_mode;

	window_pane_set_mode(wp, NULL, mode, target, args);
	return (CMD_RETURN_NORMAL);
}


// Source: cmd-choose-tree.c
// Lines 95-117
