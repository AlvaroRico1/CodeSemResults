format_cb_current_command(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;
	char			*cmd, *value;

	if (wp == NULL || wp->shell == NULL)
		return (NULL);

	cmd = osdep_get_name(wp->fd, wp->tty);
	if (cmd == NULL || *cmd == '\0') {
		free(cmd);
		cmd = cmd_stringify_argv(wp->argc, wp->argv);
		if (cmd == NULL || *cmd == '\0') {
			free(cmd);
			cmd = xstrdup(wp->shell);
		}
	}
	value = parse_window_name(cmd);
	free(cmd);
	return (value);
}


// Source: format.c
// Lines 804-824
