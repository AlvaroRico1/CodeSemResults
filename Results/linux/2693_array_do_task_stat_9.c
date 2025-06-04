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


// Source: array.c
// Lines 430-519
