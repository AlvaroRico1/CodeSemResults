cmd_rename_window_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args		*args = cmd_get_args(self);
	struct cmd_find_state	*target = cmdq_get_target(item);
	struct winlink		*wl = target->wl;
	char			*newname;

	newname = format_single_from_target(item, args_string(args, 0));
	window_set_name(wl->window, newname);
	options_set_number(wl->window->options, "automatic-rename", 0);

	server_redraw_window_borders(wl->window);
	server_status_window(wl->window);
	free(newname);

	return (CMD_RETURN_NORMAL);
}


// Source: cmd-rename-window.c
// Lines 46-62
