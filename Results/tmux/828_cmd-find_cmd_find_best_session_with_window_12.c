cmd_find_best_session_with_window(struct cmd_find_state *fs)
{
	struct session	**slist = NULL;
	u_int		  ssize;
	struct session	 *s;

	log_debug("%s: window is @%u", __func__, fs->w->id);

	ssize = 0;
	RB_FOREACH(s, sessions, &sessions) {
		if (!session_has(s, fs->w))
			continue;
		slist = xreallocarray(slist, ssize + 1, sizeof *slist);
		slist[ssize++] = s;
	}
	if (ssize == 0)
		goto fail;
	fs->s = cmd_find_best_session(slist, ssize, fs->flags);
	if (fs->s == NULL)
		goto fail;
	free(slist);
	return (cmd_find_best_winlink_with_window(fs));

fail:
	free(slist);
	return (-1);
}


// Source: cmd-find.c
// Lines 176-202
