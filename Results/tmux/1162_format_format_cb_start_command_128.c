format_cb_start_command(struct format_tree *ft)
{
	struct window_pane	*wp = ft->wp;

	if (wp == NULL)
		return (NULL);

	return (cmd_stringify_argv(wp->argc, wp->argv));
}


// Source: format.c
// Lines 792-800
