static int comm_show(struct seq_file *m, void *v)
{
	struct inode *inode = m->private;
	struct task_struct *p;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;

	proc_task_name(m, p, false);
	seq_putc(m, '\n');

	put_task_struct(p);

	return 0;
}


// Source: base.c
// Lines 1562-1577
