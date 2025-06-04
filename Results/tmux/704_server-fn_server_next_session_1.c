server_next_session(struct session *s)
{
	struct session *s_loop, *s_out = NULL;

	RB_FOREACH(s_loop, sessions, &sessions) {
		if (s_loop == s)
			continue;
		if (s_out == NULL ||
		    timercmp(&s_loop->activity_time, &s_out->activity_time, <))
			s_out = s_loop;
	}
	return (s_out);
}


// Source: server-fn.c
// Lines 403-415
