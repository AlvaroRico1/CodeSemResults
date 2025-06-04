screen_write_offset_timer(__unused int fd, __unused short events, void *data)
{
	struct window	*w = data;

	tty_update_window_offset(w);
}


// Source: screen-write.c
// Lines 79-84
