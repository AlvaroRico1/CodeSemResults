SYSCALL_DEFINE5(ppoll, struct pollfd __user *, ufds, unsigned int, nfds,
		struct __kernel_timespec __user *, tsp, const sigset_t __user *, sigmask,
		size_t, sigsetsize)
{
	struct timespec64 ts, end_time, *to = NULL;
	int ret;

	if (tsp) {
		if (get_timespec64(&ts, tsp))
			return -EFAULT;

		to = &end_time;
		if (poll_select_set_timeout(to, ts.tv_sec, ts.tv_nsec))
			return -EINVAL;
	}

	ret = set_user_sigmask(sigmask, sigsetsize);
	if (ret)
		return ret;

	ret = do_sys_poll(ufds, nfds, to);
	return poll_select_finish(&end_time, tsp, PT_TIMESPEC, ret);
}


// Source: select.c
// Lines 1081-1103
