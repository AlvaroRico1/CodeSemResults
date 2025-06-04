static ssize_t proc_reg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	ssize_t rv = -EIO;
	if (use_pde(pde)) {
		typeof_member(struct file_operations, write) write;

		write = pde->proc_fops->write;
		if (write)
			rv = write(file, buf, count, ppos);
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 229-240
