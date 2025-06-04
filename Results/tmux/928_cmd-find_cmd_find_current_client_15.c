cmd_find_current_client(struct cmdq_item *item, int quiet)
{
	struct client		*c = NULL, *found;
	struct session		*s;
	struct window_pane	*wp;
	struct cmd_find_state	 fs;

	if (item != NULL)
		c = cmdq_get_client(item);
	if (c != NULL && c->session != NULL)
		return (c);

	found = NULL;
	if (c != NULL && (wp = cmd_find_inside_pane(c)) != NULL) {
		cmd_find_clear_state(&fs, CMD_FIND_QUIET);
		fs.w = wp->window;
		if (cmd_find_best_session_with_window(&fs) == 0)
			found = cmd_find_best_client(fs.s);
	} else {
		s = cmd_find_best_session(NULL, 0, CMD_FIND_QUIET);
		if (s != NULL)
			found = cmd_find_best_client(s);
	}
	if (found == NULL && item != NULL && !quiet)
		cmdq_error(item, "no current client");
	log_debug("%s: no target, return %p", __func__, found);
	return (found);
}


// Source: cmd-find.c
// Lines 1243-1270
