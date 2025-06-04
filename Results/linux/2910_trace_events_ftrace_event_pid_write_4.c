ftrace_event_pid_write(struct file *filp, const char __user *ubuf,
		       size_t cnt, loff_t *ppos)
{
	struct seq_file *m = filp->private_data;
	struct trace_array *tr = m->private;
	struct trace_pid_list *filtered_pids = NULL;
	struct trace_pid_list *pid_list;
	struct trace_event_file *file;
	ssize_t ret;

	if (!cnt)
		return 0;

	ret = tracing_update_buffers();
	if (ret < 0)
		return ret;

	mutex_lock(&event_mutex);

	filtered_pids = rcu_dereference_protected(tr->filtered_pids,
					     lockdep_is_held(&event_mutex));

	ret = trace_pid_write(filtered_pids, &pid_list, ubuf, cnt);
	if (ret < 0)
		goto out;

	rcu_assign_pointer(tr->filtered_pids, pid_list);

	list_for_each_entry(file, &tr->events, list) {
		set_bit(EVENT_FILE_FL_PID_FILTER_BIT, &file->flags);
	}

	if (filtered_pids) {
		tracepoint_synchronize_unregister();
		trace_free_pid_list(filtered_pids);
	} else if (pid_list) {
		/*
		 * Register a probe that is called before all other probes
		 * to set ignore_pid if next or prev do not match.
		 * Register a probe this is called after all other probes
		 * to only keep ignore_pid set if next pid matches.
		 */
		register_trace_prio_sched_switch(event_filter_pid_sched_switch_probe_pre,
						 tr, INT_MAX);
		register_trace_prio_sched_switch(event_filter_pid_sched_switch_probe_post,
						 tr, 0);

		register_trace_prio_sched_wakeup(event_filter_pid_sched_wakeup_probe_pre,
						 tr, INT_MAX);
		register_trace_prio_sched_wakeup(event_filter_pid_sched_wakeup_probe_post,
						 tr, 0);

		register_trace_prio_sched_wakeup_new(event_filter_pid_sched_wakeup_probe_pre,
						     tr, INT_MAX);
		register_trace_prio_sched_wakeup_new(event_filter_pid_sched_wakeup_probe_post,
						     tr, 0);

		register_trace_prio_sched_waking(event_filter_pid_sched_wakeup_probe_pre,
						 tr, INT_MAX);
		register_trace_prio_sched_waking(event_filter_pid_sched_wakeup_probe_post,
						 tr, 0);
	}

	/*
	 * Ignoring of pids is done at task switch. But we have to
	 * check for those tasks that are currently running.
	 * Always do this in case a pid was appended or removed.
	 */
	on_each_cpu(ignore_task_cpu, tr, 1);

 out:
	mutex_unlock(&event_mutex);

	if (ret > 0)
		*ppos += ret;

	return ret;
}


// Source: trace_events.c
// Lines 1583-1660
