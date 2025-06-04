static int __mem_open(struct inode *inode, struct file *file, unsigned int mode)
{
	struct mm_struct *mm = proc_mem_open(inode, mode);

	if (IS_ERR(mm))
		return PTR_ERR(mm);

	file->private_data = mm;
	return 0;
}


// Source: base.c
// Lines 795-804
