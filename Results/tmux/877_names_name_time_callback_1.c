name_time_callback(__unused int fd, __unused short events, void *arg)
{
	struct window	*w = arg;

	/* The event loop will call check_window_name for us on the way out. */
	log_debug("@%u name timer expired", w->id);
}


// Source: names.c
// Lines 34-40
