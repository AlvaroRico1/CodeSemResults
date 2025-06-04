static struct task_struct *first_tid(struct pid *pid, int tid, loff_t f_pos,
					struct pid_namespace *ns)
{
	struct task_struct *pos, *task;
	unsigned long nr = f_pos;

	if (nr != f_pos)	/* 32bit overflow? */
		return NULL;

	rcu_read_lock();
	task = pid_task(pid, PIDTYPE_PID);
	if (!task)
		goto fail;

	/* Attempt to start with the tid of a thread */
	if (tid && nr) {
		pos = find_task_by_pid_ns(tid, ns);
		if (pos && same_thread_group(pos, task))
			goto found;
	}

	/* If nr exceeds the number of threads there is nothing todo */
	if (nr >= get_nr_threads(task))
		goto fail;

	/* If we haven't found our starting place yet start
	 * with the leader and walk nr threads forward.
	 */
	pos = task = task->group_leader;
	do {
		if (!nr--)
			goto found;
	} while_each_thread(task, pos);
fail:
	pos = NULL;
	goto out;
found:
	get_task_struct(pos);
out:
	rcu_read_unlock();
	return pos;
}


// Source: base.c
// Lines 3583-3624
