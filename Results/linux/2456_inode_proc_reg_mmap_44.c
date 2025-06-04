static int proc_reg_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct proc_dir_entry *pde = PDE(file_inode(file));
	int rv = -EIO;
	if (use_pde(pde)) {
		typeof_member(struct file_operations, mmap) mmap;

		mmap = pde->proc_fops->mmap;
		if (mmap)
			rv = mmap(file, vma);
		unuse_pde(pde);
	}


// Source: inode.c
// Lines 291-302
