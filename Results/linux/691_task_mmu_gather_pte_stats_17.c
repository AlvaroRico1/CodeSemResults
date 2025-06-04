static int gather_pte_stats(pmd_t *pmd, unsigned long addr,
		unsigned long end, struct mm_walk *walk)
{
	struct numa_maps *md = walk->private;
	struct vm_area_struct *vma = walk->vma;
	spinlock_t *ptl;
	pte_t *orig_pte;
	pte_t *pte;

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	ptl = pmd_trans_huge_lock(pmd, vma);
	if (ptl) {
		struct page *page;

		page = can_gather_numa_stats_pmd(*pmd, vma, addr);
		if (page)
			gather_stats(page, md, pmd_dirty(*pmd),
				     HPAGE_PMD_SIZE/PAGE_SIZE);
		spin_unlock(ptl);
		return 0;
	}

	if (pmd_trans_unstable(pmd))
		return 0;
#endif
	orig_pte = pte = pte_offset_map_lock(walk->mm, pmd, addr, &ptl);
	do {
		struct page *page = can_gather_numa_stats(*pte, vma, addr);
		if (!page)
			continue;
		gather_stats(page, md, pte_dirty(*pte), 1);

	} while (pte++, addr += PAGE_SIZE, addr != end);
	pte_unmap_unlock(orig_pte, ptl);
	cond_resched();
	return 0;
}


// Source: task_mmu.c
// Lines 1736-1772
