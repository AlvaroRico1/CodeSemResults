static ssize_t pagemap_read(struct file *file, char __user *buf,
			    size_t count, loff_t *ppos)
{
	struct mm_struct *mm = file->private_data;
	struct pagemapread pm;
	struct mm_walk pagemap_walk = {};
	unsigned long src;
	unsigned long svpfn;
	unsigned long start_vaddr;
	unsigned long end_vaddr;
	int ret = 0, copied = 0;

	if (!mm || !mmget_not_zero(mm))
		goto out;

	ret = -EINVAL;
	/* file position must be aligned */
	if ((*ppos % PM_ENTRY_BYTES) || (count % PM_ENTRY_BYTES))
		goto out_mm;

	ret = 0;
	if (!count)
		goto out_mm;

	/* do not disclose physical addresses: attack vector */
	pm.show_pfn = file_ns_capable(file, &init_user_ns, CAP_SYS_ADMIN);

	pm.len = (PAGEMAP_WALK_SIZE >> PAGE_SHIFT);
	pm.buffer = kmalloc_array(pm.len, PM_ENTRY_BYTES, GFP_KERNEL);
	ret = -ENOMEM;
	if (!pm.buffer)
		goto out_mm;

	pagemap_walk.pmd_entry = pagemap_pmd_range;
	pagemap_walk.pte_hole = pagemap_pte_hole;
#ifdef CONFIG_HUGETLB_PAGE
	pagemap_walk.hugetlb_entry = pagemap_hugetlb_range;
#endif
	pagemap_walk.mm = mm;
	pagemap_walk.private = &pm;

	src = *ppos;
	svpfn = src / PM_ENTRY_BYTES;
	start_vaddr = svpfn << PAGE_SHIFT;
	end_vaddr = mm->task_size;

	/* watch out for wraparound */
	if (svpfn > mm->task_size >> PAGE_SHIFT)
		start_vaddr = end_vaddr;

	/*
	 * The odds are that this will stop walking way
	 * before end_vaddr, because the length of the
	 * user buffer is tracked in "pm", and the walk
	 * will stop when we hit the end of the buffer.
	 */
	ret = 0;
	while (count && (start_vaddr < end_vaddr)) {
		int len;
		unsigned long end;

		pm.pos = 0;
		end = (start_vaddr + PAGEMAP_WALK_SIZE) & PAGEMAP_WALK_MASK;
		/* overflow ? */
		if (end < start_vaddr || end > end_vaddr)
			end = end_vaddr;
		ret = down_read_killable(&mm->mmap_sem);
		if (ret)
			goto out_free;
		ret = walk_page_range(start_vaddr, end, &pagemap_walk);
		up_read(&mm->mmap_sem);
		start_vaddr = end;

		len = min(count, PM_ENTRY_BYTES * pm.pos);
		if (copy_to_user(buf, pm.buffer, len)) {
			ret = -EFAULT;
			goto out_free;
		}
		copied += len;
		buf += len;
		count -= len;
	}
	*ppos += copied;
	if (!ret || ret == PM_END_OF_BUFFER)
		ret = copied;

out_free:
	kfree(pm.buffer);
out_mm:
	mmput(mm);
out:
	return ret;
}


// Source: task_mmu.c
// Lines 1520-1612
