static long proc_reg_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	long rv = -ENOTTY;
	if (use_pde(pde)) {
		typeof_member(struct file_operations, compat_ioctl) compat_ioctl;

		compat_ioctl = pde->proc_fops->compat_ioctl;
		if (compat_ioctl)
			rv = compat_ioctl(file, cmd, arg);
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 275-286
