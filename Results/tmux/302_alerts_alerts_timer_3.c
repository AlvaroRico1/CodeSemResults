alerts_timer(__unused int fd, __unused short events, void *arg)
{
	struct window	*w = arg;

	log_debug("@%u alerts timer expired", w->id);
	alerts_queue(w, WINDOW_SILENCE);
}


// Source: alerts.c
// Lines 43-49
