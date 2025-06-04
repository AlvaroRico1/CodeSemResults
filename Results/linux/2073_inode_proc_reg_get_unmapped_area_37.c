proc_reg_get_unmapped_area(struct file *file, unsigned long orig_addr,
			   unsigned long len, unsigned long pgoff,
			   unsigned long flags)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	unsigned long rv = -EIO;

	if (use_pde(pde)) {
		typeof_member(struct file_operations, get_unmapped_area) get_area;

		get_area = pde->proc_fops->get_unmapped_area;
#ifdef CONFIG_MMU
		if (!get_area)
			get_area = current->mm->get_unmapped_area;
#endif

		if (get_area)
			rv = get_area(file, orig_addr, len, pgoff, flags);
		else
			rv = orig_addr;
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 307-328
