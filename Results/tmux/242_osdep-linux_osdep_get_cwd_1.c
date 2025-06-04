osdep_get_cwd(int fd)
{
	static char	 target[MAXPATHLEN + 1];
	char		*path;
	pid_t		 pgrp, sid;
	ssize_t		 n;

	if ((pgrp = tcgetpgrp(fd)) == -1)
		return (NULL);

	xasprintf(&path, "/proc/%lld/cwd", (long long) pgrp);
	n = readlink(path, target, MAXPATHLEN);
	free(path);

	if (n == -1 && ioctl(fd, TIOCGSID, &sid) != -1) {
		xasprintf(&path, "/proc/%lld/cwd", (long long) sid);
		n = readlink(path, target, MAXPATHLEN);
		free(path);
	}

	if (n > 0) {
		target[n] = '\0';
		return (target);
	}
	return (NULL);
}


// Source: osdep-linux.c
// Lines 64-89
