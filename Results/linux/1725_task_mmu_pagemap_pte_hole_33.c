static int pagemap_pte_hole(unsigned long start, unsigned long end,
				struct mm_walk *walk)
{
	struct pagemapread *pm = walk->private;
	unsigned long addr = start;
	int err = 0;

	while (addr < end) {
		struct vm_area_struct *vma = find_vma(walk->mm, addr);
		pagemap_entry_t pme = make_pme(0, 0);
		/* End of address space hole, which we mark as non-present. */
		unsigned long hole_end;

		if (vma)
			hole_end = min(end, vma->vm_start);
		else
			hole_end = end;

		for (; addr < hole_end; addr += PAGE_SIZE) {
			err = add_to_pagemap(addr, &pme, pm);
			if (err)
				goto out;
		}

		if (!vma)
			break;

		/* Addresses in the VMA. */
		if (vma->vm_flags & VM_SOFTDIRTY)
			pme = make_pme(0, PM_SOFT_DIRTY);
		for (; addr < min(end, vma->vm_end); addr += PAGE_SIZE) {
			err = add_to_pagemap(addr, &pme, pm);
			if (err)
				goto out;
		}
	}
out:
	return err;
}


// Source: task_mmu.c
// Lines 1276-1314
