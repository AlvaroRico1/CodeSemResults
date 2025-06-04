static int pagemap_release(struct inode *inode, struct file *file)
{
	struct mm_struct *mm = file->private_data;

	if (mm)
		mmdrop(mm);
	return 0;
}


// Source: task_mmu.c
// Lines 1625-1632
