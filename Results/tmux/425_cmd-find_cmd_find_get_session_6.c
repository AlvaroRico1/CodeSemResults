cmd_find_get_session(struct cmd_find_state *fs, const char *session)
{
	struct session	*s, *s_loop;
	struct client	*c;

	log_debug("%s: %s", __func__, session);

	/* Check for session ids starting with $. */
	if (*session == '$') {
		fs->s = session_find_by_id_str(session);
		if (fs->s == NULL)
			return (-1);
		return (0);
	}

	/* Look for exactly this session. */
	fs->s = session_find(session);
	if (fs->s != NULL)
		return (0);

	/* Look for as a client. */
	c = cmd_find_client(NULL, session, 1);
	if (c != NULL && c->session != NULL) {
		fs->s = c->session;
		return (0);
	}

	/* Stop now if exact only. */
	if (fs->flags & CMD_FIND_EXACT_SESSION)
		return (-1);

	/* Otherwise look for prefix. */
	s = NULL;
	RB_FOREACH(s_loop, sessions, &sessions) {
		if (strncmp(session, s_loop->name, strlen(session)) == 0) {
			if (s != NULL)
				return (-1);
			s = s_loop;
		}
	}
	if (s != NULL) {
		fs->s = s;
		return (0);
	}

	/* Then as a pattern. */
	s = NULL;
	RB_FOREACH(s_loop, sessions, &sessions) {
		if (fnmatch(session, s_loop->name, 0) == 0) {
			if (s != NULL)
				return (-1);
			s = s_loop;
		}
	}
	if (s != NULL) {
		fs->s = s;
		return (0);
	}

	return (-1);
}


// Source: cmd-find.c
// Lines 248-308
