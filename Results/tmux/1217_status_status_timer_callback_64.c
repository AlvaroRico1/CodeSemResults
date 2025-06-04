status_timer_callback(__unused int fd, __unused short events, void *arg)
{
	struct client	*c = arg;
	struct session	*s = c->session;
	struct timeval	 tv;

	evtimer_del(&c->status.timer);

	if (s == NULL)
		return;

	if (c->message_string == NULL && c->prompt_string == NULL)
		c->flags |= CLIENT_REDRAWSTATUS;

	timerclear(&tv);
	tv.tv_sec = options_get_number(s->options, "status-interval");

	if (tv.tv_sec != 0)
		evtimer_add(&c->status.timer, &tv);
	log_debug("client %p, status interval %d", c, (int)tv.tv_sec);
}


// Source: status.c
// Lines 180-200
