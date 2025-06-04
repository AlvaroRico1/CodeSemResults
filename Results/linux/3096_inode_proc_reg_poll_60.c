static __poll_t proc_reg_poll(struct file *file, struct poll_table_struct *pts)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	__poll_t rv = DEFAULT_POLLMASK;
	if (use_pde(pde)) {
		typeof_member(struct file_operations, poll) poll;

		poll = pde->proc_fops->poll;
		if (poll)
			rv = poll(file, pts);
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 244-255
