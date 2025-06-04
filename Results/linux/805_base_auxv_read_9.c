static ssize_t auxv_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	struct mm_struct *mm = file->private_data;
	unsigned int nwords = 0;

	if (!mm)
		return 0;
	do {
		nwords += 2;
	} while (mm->saved_auxv[nwords - 2] != 0); /* AT_NULL */
	return simple_read_from_buffer(buf, count, ppos, mm->saved_auxv,
				       nwords * sizeof(mm->saved_auxv[0]));
}


// Source: base.c
// Lines 995-1008
