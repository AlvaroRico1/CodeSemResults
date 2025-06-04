int p_write(git_file fd, const void *buf, size_t cnt)
{
	const char *b = buf;

	while (cnt) {
		ssize_t r;
#ifdef GIT_WIN32
		GIT_ASSERT((size_t)((unsigned int)cnt) == cnt);
		r = write(fd, b, (unsigned int)cnt);
#else
		r = write(fd, b, cnt);
#endif
		if (r < 0) {
			if (errno == EINTR || GIT_ISBLOCKED(errno))
				continue;
			return -1;
		}
		if (!r) {
			errno = EPIPE;
			return -1;
		}
		cnt -= r;
		b += r;
	}
	return 0;
}


// Source: posix.c
// Lines 200-225
