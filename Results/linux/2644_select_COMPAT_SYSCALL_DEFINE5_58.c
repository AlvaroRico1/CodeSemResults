COMPAT_SYSCALL_DEFINE5(ppoll_time32, struct pollfd __user *, ufds,
	unsigned int,  nfds, struct old_timespec32 __user *, tsp,
	const compat_sigset_t __user *, sigmask, compat_size_t, sigsetsize)
{
	struct timespec64 ts, end_time, *to = NULL;
	int ret;

	if (tsp) {
		if (get_old_timespec32(&ts, tsp))
			return -EFAULT;

		to = &end_time;
		if (poll_select_set_timeout(to, ts.tv_sec, ts.tv_nsec))
			return -EINVAL;
	}

	ret = set_compat_user_sigmask(sigmask, sigsetsize);
	if (ret)
		return ret;

	ret = do_sys_poll(ufds, nfds, to);
	return poll_select_finish(&end_time, tsp, PT_OLD_TIMESPEC, ret);
}


// Source: select.c
// Lines 1373-1395
