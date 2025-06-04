static long do_restart_poll(struct restart_block *restart_block)
{
	struct pollfd __user *ufds = restart_block->poll.ufds;
	int nfds = restart_block->poll.nfds;
	struct timespec64 *to = NULL, end_time;
	int ret;

	if (restart_block->poll.has_timeout) {
		end_time.tv_sec = restart_block->poll.tv_sec;
		end_time.tv_nsec = restart_block->poll.tv_nsec;
		to = &end_time;
	}

	ret = do_sys_poll(ufds, nfds, to);

	if (ret == -ERESTARTNOHAND) {
		restart_block->fn = do_restart_poll;
		ret = -ERESTART_RESTARTBLOCK;
	}
	return ret;
}


// Source: select.c
// Lines 1025-1045
