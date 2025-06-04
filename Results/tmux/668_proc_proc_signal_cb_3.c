proc_signal_cb(int signo, __unused short events, void *arg)
{
	struct tmuxproc	*tp = arg;

	tp->signalcb(signo);
}


// Source: proc.c
// Lines 122-127
