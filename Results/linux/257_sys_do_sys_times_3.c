static void do_sys_times(struct tms *tms)
{
	u64 tgutime, tgstime, cutime, cstime;

	thread_group_cputime_adjusted(current, &tgutime, &tgstime);
	cutime = current->signal->cutime;
	cstime = current->signal->cstime;
	tms->tms_utime = nsec_to_clock_t(tgutime);
	tms->tms_stime = nsec_to_clock_t(tgstime);
	tms->tms_cutime = nsec_to_clock_t(cutime);
	tms->tms_cstime = nsec_to_clock_t(cstime);
}


// Source: sys.c
// Lines 943-954
