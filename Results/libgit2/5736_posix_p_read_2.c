ssize_t p_read(git_file fd, void *buf, size_t cnt)
{
	char *b = buf;

	if (!git__is_ssizet(cnt)) {
#ifdef GIT_WIN32
		SetLastError(ERROR_INVALID_PARAMETER);
#endif
		errno = EINVAL;
		return -1;
	}

	while (cnt) {
		ssize_t r;
#ifdef GIT_WIN32
		r = read(fd, b, cnt > INT_MAX ? INT_MAX : (unsigned int)cnt);
#else
		r = read(fd, b, cnt);
#endif
		if (r < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}
		if (!r)
			break;
		cnt -= r;
		b += r;
	}
	return (b - (char *)buf);
}


// Source: posix.c
// Lines 168-198
