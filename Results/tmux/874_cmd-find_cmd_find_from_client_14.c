cmd_find_from_client(struct cmd_find_state *fs, struct client *c, int flags)
{
	struct window_pane	*wp;

	/* If no client, treat as from nothing. */
	if (c == NULL)
		return (cmd_find_from_nothing(fs, flags));

	/* If this is an attached client, all done. */
	if (c->session != NULL) {
		cmd_find_clear_state(fs, flags);

		fs->wp = server_client_get_pane(c);
		if (fs->wp == NULL) {
			cmd_find_from_session(fs, c->session, flags);
			return (0);
		}
		fs->s = c->session;
		fs->wl = fs->s->curw;
		fs->w = fs->wl->window;

		cmd_find_log_state(__func__, fs);
		return (0);
	}
	cmd_find_clear_state(fs, flags);

	/*
	 * If this is an unattached client running in a pane, we can use that
	 * to limit the list of sessions to those containing that pane.
	 */
	wp = cmd_find_inside_pane(c);
	if (wp == NULL)
		goto unknown_pane;

	/*
	 * Don't have a session, or it doesn't have this pane. Try all
	 * sessions.
	 */
	fs->w = wp->window;
	if (cmd_find_best_session_with_window(fs) != 0) {
		/*
		 * The window may have been destroyed but the pane
		 * still on all_window_panes due to something else
		 * holding a reference.
		 */
		goto unknown_pane;
	}
	fs->wl = fs->s->curw;
	fs->w = fs->wl->window;
	fs->wp = fs->w->active; /* use active pane */

	cmd_find_log_state(__func__, fs);
	return (0);

unknown_pane:
	/* We can't find the pane so need to guess. */
	return (cmd_find_from_nothing(fs, flags));
}


// Source: cmd-find.c
// Lines 859-916
