static int do_task_stat(struct seq_file *m, struct pid_namespace *ns,
			struct pid *pid, struct task_struct *task, int whole)
{
	unsigned long vsize, eip, esp, wchan = 0;
	int priority, nice;
	int tty_pgrp = -1, tty_nr = 0;
	sigset_t sigign, sigcatch;
	char state;
	pid_t ppid = 0, pgid = -1, sid = -1;
	int num_threads = 0;
	int permitted;
	struct mm_struct *mm;
	unsigned long long start_time;
	unsigned long cmin_flt = 0, cmaj_flt = 0;
	unsigned long  min_flt = 0,  maj_flt = 0;
	u64 cutime, cstime, utime, stime;
	u64 cgtime, gtime;
	unsigned long rsslim = 0;
	unsigned long flags;

	state = *get_task_state(task);
	vsize = eip = esp = 0;
	permitted = ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS | PTRACE_MODE_NOAUDIT);
	mm = get_task_mm(task);
	if (mm) {
		vsize = task_vsize(mm);
		/*
		 * esp and eip are intentionally zeroed out.  There is no
		 * non-racy way to read them without freezing the task.
		 * Programs that need reliable values can use ptrace(2).
		 *
		 * The only exception is if the task is core dumping because
		 * a program is not able to use ptrace(2) in that case. It is
		 * safe because the task has stopped executing permanently.
		 */
		if (permitted && (task->flags & (PF_EXITING|PF_DUMPCORE))) {
			if (try_get_task_stack(task)) {
				eip = KSTK_EIP(task);
				esp = KSTK_ESP(task);
				put_task_stack(task);
			}
		}
	}

	sigemptyset(&sigign);
	sigemptyset(&sigcatch);
	cutime = cstime = utime = stime = 0;
	cgtime = gtime = 0;

	if (lock_task_sighand(task, &flags)) {
		struct signal_struct *sig = task->signal;

		if (sig->tty) {
			struct pid *pgrp = tty_get_pgrp(sig->tty);
			tty_pgrp = pid_nr_ns(pgrp, ns);
			put_pid(pgrp);
			tty_nr = new_encode_dev(tty_devnum(sig->tty));
		}

		num_threads = get_nr_threads(task);
		collect_sigign_sigcatch(task, &sigign, &sigcatch);

		cmin_flt = sig->cmin_flt;
		cmaj_flt = sig->cmaj_flt;
		cutime = sig->cutime;
		cstime = sig->cstime;
		cgtime = sig->cgtime;
		rsslim = READ_ONCE(sig->rlim[RLIMIT_RSS].rlim_cur);

		/* add up live thread stats at the group level */
		if (whole) {
			struct task_struct *t = task;
			do {
				min_flt += t->min_flt;
				maj_flt += t->maj_flt;
				gtime += task_gtime(t);
			} while_each_thread(task, t);

			min_flt += sig->min_flt;
			maj_flt += sig->maj_flt;
			thread_group_cputime_adjusted(task, &utime, &stime);
			gtime += sig->gtime;
		}

		sid = task_session_nr_ns(task, ns);
		ppid = task_tgid_nr_ns(task->real_parent, ns);
		pgid = task_pgrp_nr_ns(task, ns);

		unlock_task_sighand(task, &flags);
	}

	if (permitted && (!whole || num_threads < 2))
		wchan = get_wchan(task);
	if (!whole) {
		min_flt = task->min_flt;
		maj_flt = task->maj_flt;
		task_cputime_adjusted(task, &utime, &stime);
		gtime = task_gtime(task);
	}

	/* scale priority and nice values from timeslices to -20..20 */
	/* to make it look like a "normal" Unix priority/nice value  */
	priority = task_prio(task);
	nice = task_nice(task);

	/* convert nsec -> ticks */
	start_time = nsec_to_clock_t(task->real_start_time);

	seq_put_decimal_ull(m, "", pid_nr_ns(pid, ns));
	seq_puts(m, " (");
	proc_task_name(m, task, false);
	seq_puts(m, ") ");
	seq_putc(m, state);
	seq_put_decimal_ll(m, " ", ppid);
	seq_put_decimal_ll(m, " ", pgid);
	seq_put_decimal_ll(m, " ", sid);
	seq_put_decimal_ll(m, " ", tty_nr);
	seq_put_decimal_ll(m, " ", tty_pgrp);
	seq_put_decimal_ull(m, " ", task->flags);
	seq_put_decimal_ull(m, " ", min_flt);
	seq_put_decimal_ull(m, " ", cmin_flt);
	seq_put_decimal_ull(m, " ", maj_flt);
	seq_put_decimal_ull(m, " ", cmaj_flt);
	seq_put_decimal_ull(m, " ", nsec_to_clock_t(utime));
	seq_put_decimal_ull(m, " ", nsec_to_clock_t(stime));
	seq_put_decimal_ll(m, " ", nsec_to_clock_t(cutime));
	seq_put_decimal_ll(m, " ", nsec_to_clock_t(cstime));
	seq_put_decimal_ll(m, " ", priority);
	seq_put_decimal_ll(m, " ", nice);
	seq_put_decimal_ll(m, " ", num_threads);
	seq_put_decimal_ull(m, " ", 0);
	seq_put_decimal_ull(m, " ", start_time);
	seq_put_decimal_ull(m, " ", vsize);
	seq_put_decimal_ull(m, " ", mm ? get_mm_rss(mm) : 0);
	seq_put_decimal_ull(m, " ", rsslim);
	seq_put_decimal_ull(m, " ", mm ? (permitted ? mm->start_code : 1) : 0);
	seq_put_decimal_ull(m, " ", mm ? (permitted ? mm->end_code : 1) : 0);
	seq_put_decimal_ull(m, " ", (permitted && mm) ? mm->start_stack : 0);
	seq_put_decimal_ull(m, " ", esp);
	seq_put_decimal_ull(m, " ", eip);
	/* The signal information here is obsolete.
	 * It must be decimal for Linux 2.0 compatibility.
	 * Use /proc/#/status for real-time signals.
	 */
	seq_put_decimal_ull(m, " ", task->pending.signal.sig[0] & 0x7fffffffUL);
	seq_put_decimal_ull(m, " ", task->blocked.sig[0] & 0x7fffffffUL);
	seq_put_decimal_ull(m, " ", sigign.sig[0] & 0x7fffffffUL);
	seq_put_decimal_ull(m, " ", sigcatch.sig[0] & 0x7fffffffUL);

	/*
	 * We used to output the absolute kernel address, but that's an
	 * information leak - so instead we show a 0/1 flag here, to signal
	 * to user-space whether there's a wchan field in /proc/PID/wchan.
	 *
	 * This works with older implementations of procps as well.
	 */
	if (wchan)
		seq_puts(m, " 1");
	else
		seq_puts(m, " 0");

	seq_put_decimal_ull(m, " ", 0);
	seq_put_decimal_ull(m, " ", 0);
	seq_put_decimal_ll(m, " ", task->exit_signal);
	seq_put_decimal_ll(m, " ", task_cpu(task));
	seq_put_decimal_ull(m, " ", task->rt_priority);
	seq_put_decimal_ull(m, " ", task->policy);
	seq_put_decimal_ull(m, " ", delayacct_blkio_ticks(task));
	seq_put_decimal_ull(m, " ", nsec_to_clock_t(gtime));
	seq_put_decimal_ll(m, " ", nsec_to_clock_t(cgtime));

	if (mm && permitted) {
		seq_put_decimal_ull(m, " ", mm->start_data);
		seq_put_decimal_ull(m, " ", mm->end_data);
		seq_put_decimal_ull(m, " ", mm->start_brk);
		seq_put_decimal_ull(m, " ", mm->arg_start);
		seq_put_decimal_ull(m, " ", mm->arg_end);
		seq_put_decimal_ull(m, " ", mm->env_start);
		seq_put_decimal_ull(m, " ", mm->env_end);
	} else
		seq_puts(m, " 0 0 0 0 0 0 0");

	if (permitted)
		seq_put_decimal_ll(m, " ", task->exit_code);
	else
		seq_puts(m, " 0");

	seq_putc(m, '\n');
	if (mm)
		mmput(mm);
	return 0;
}


// Source: array.c
// Lines 430-621
