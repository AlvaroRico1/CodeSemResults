static int do_compat_select(int n, compat_ulong_t __user *inp,
	compat_ulong_t __user *outp, compat_ulong_t __user *exp,
	struct old_timeval32 __user *tvp)
{
	struct timespec64 end_time, *to = NULL;
	struct old_timeval32 tv;
	int ret;

	if (tvp) {
		if (copy_from_user(&tv, tvp, sizeof(tv)))
			return -EFAULT;

		to = &end_time;
		if (poll_select_set_timeout(to,
				tv.tv_sec + (tv.tv_usec / USEC_PER_SEC),
				(tv.tv_usec % USEC_PER_SEC) * NSEC_PER_USEC))
			return -EINVAL;
	}

	ret = compat_core_sys_select(n, inp, outp, exp, to);
	return poll_select_finish(&end_time, tvp, PT_OLD_TIMEVAL, ret);
}


// Source: select.c
// Lines 1245-1266
