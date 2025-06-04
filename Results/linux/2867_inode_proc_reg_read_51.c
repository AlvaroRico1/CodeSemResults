static ssize_t proc_reg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	ssize_t rv = -EIO;
	if (use_pde(pde)) {
		typeof_member(struct file_operations, read) read;

		read = pde->proc_fops->read;
		if (read)
			rv = read(file, buf, count, ppos);
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 214-225
