static int timerslack_ns_show(struct seq_file *m, void *v)
{
	struct inode *inode = m->private;
	struct task_struct *p;
	int err = 0;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;

	if (p != current) {
		rcu_read_lock();
		if (!ns_capable(__task_cred(p)->user_ns, CAP_SYS_NICE)) {
			rcu_read_unlock();
			err = -EPERM;
			goto out;
		}
		rcu_read_unlock();

		err = security_task_getscheduler(p);
		if (err)
			goto out;
	}

	task_lock(p);
	seq_printf(m, "%llu\n", p->timer_slack_ns);
	task_unlock(p);

out:
	put_task_struct(p);

	return err;
}


// Source: base.c
// Lines 2405-2437
