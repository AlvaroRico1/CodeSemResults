_rl_event_read_char(EditLine *el, wchar_t *wc)
{
	char	ch;
	int	n;
	ssize_t num_read = 0;

	ch = '\0';
	*wc = L'\0';
	while (rl_event_hook) {

		(*rl_event_hook)();

#if defined(FIONREAD)
		if (ioctl(el->el_infd, FIONREAD, &n) < 0)
			return -1;
		if (n)
			num_read = read(el->el_infd, &ch, (size_t)1);
		else
			num_read = 0;
#elif defined(F_SETFL) && defined(O_NDELAY)
		if ((n = fcntl(el->el_infd, F_GETFL, 0)) < 0)
			return -1;
		if (fcntl(el->el_infd, F_SETFL, n|O_NDELAY) < 0)
			return -1;
		num_read = read(el->el_infd, &ch, 1);
		if (fcntl(el->el_infd, F_SETFL, n))
			return -1;
#else
		/* not non-blocking, but what you gonna do? */
		num_read = read(el->el_infd, &ch, 1);
		return -1;
#endif

		if (num_read < 0 && errno == EAGAIN)
			continue;
		if (num_read == 0)
			continue;
		break;
	}
	if (!rl_event_hook)
		el_set(el, EL_GETCFN, EL_BUILTIN_GETCFN);
	*wc = (wchar_t)ch;
	return (int)num_read;
}


// Source: readline.c
// Lines 2182-2225
