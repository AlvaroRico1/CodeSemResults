session_lock_timer(__unused int fd, __unused short events, void *arg)
{
	struct session	*s = arg;

	if (s->attached == 0)
		return;

	log_debug("session %s locked, activity time %lld", s->name,
	    (long long)s->activity_time.tv_sec);

	server_lock_session(s);
	recalculate_sizes();
}


// Source: session.c
// Lines 255-267
