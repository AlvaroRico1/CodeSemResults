cmd_find_best_session(struct session **slist, u_int ssize, int flags)
{
	struct session	 *s_loop, *s;
	u_int		  i;

	log_debug("%s: %u sessions to try", __func__, ssize);

	s = NULL;
	if (slist != NULL) {
		for (i = 0; i < ssize; i++) {
			if (cmd_find_session_better(slist[i], s, flags))
				s = slist[i];
		}
	} else {
		RB_FOREACH(s_loop, sessions, &sessions) {
			if (cmd_find_session_better(s_loop, s, flags))
				s = s_loop;
		}
	}
	return (s);
}


// Source: cmd-find.c
// Lines 152-172
