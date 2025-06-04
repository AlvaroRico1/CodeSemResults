tty_keys_callback(__unused int fd, __unused short events, void *data)
{
	struct tty	*tty = data;

	if (tty->flags & TTY_TIMER) {
		while (tty_keys_next(tty))
			;
	}
}


// Source: tty-keys.c
// Lines 866-874
