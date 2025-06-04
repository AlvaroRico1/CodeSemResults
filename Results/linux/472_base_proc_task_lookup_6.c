static struct dentry *proc_task_lookup(struct inode *dir, struct dentry * dentry, unsigned int flags)
{
	struct task_struct *task;
	struct task_struct *leader = get_proc_task(dir);
	unsigned tid;
	struct pid_namespace *ns;
	struct dentry *result = ERR_PTR(-ENOENT);

	if (!leader)
		goto out_no_task;

	tid = name_to_int(&dentry->d_name);
	if (tid == ~0U)
		goto out;

	ns = dentry->d_sb->s_fs_info;
	rcu_read_lock();
	task = find_task_by_pid_ns(tid, ns);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		goto out;
	if (!same_thread_group(leader, task))
		goto out_drop_task;

	result = proc_task_instantiate(dentry, task, NULL);
out_drop_task:
	put_task_struct(task);
out:
	put_task_struct(leader);
out_no_task:
	return result;
}


// Source: base.c
// Lines 3536-3569
