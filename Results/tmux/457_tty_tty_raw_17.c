tty_raw(struct tty *tty, const char *s)
{
	struct client	*c = tty->client;
	ssize_t		 n, slen;
	u_int		 i;

	slen = strlen(s);
	for (i = 0; i < 5; i++) {
		n = write(c->fd, s, slen);
		if (n >= 0) {
			s += n;
			slen -= n;
			if (slen == 0)
				break;
		} else if (n == -1 && errno != EAGAIN)
			break;
		usleep(100);
	}
}


// Source: tty.c
// Lines 476-494
