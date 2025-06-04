input_timer_callback(__unused int fd, __unused short events, void *arg)
{
	struct input_ctx	*ictx = arg;

	log_debug("%s: %s expired" , __func__, ictx->state->name);
	input_reset(ictx, 0);
}


// Source: input.c
// Lines 741-747
