void cl_perf_timer__stop(cl_perf_timer *t)
{
	double time_now = git__timer();

	t->last = time_now - t->time_started;
	t->sum += t->last;
}


// Source: clar_libgit2_timer.c
// Lines 14-20
