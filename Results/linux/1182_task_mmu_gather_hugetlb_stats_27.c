static int gather_hugetlb_stats(pte_t *pte, unsigned long hmask,
		unsigned long addr, unsigned long end, struct mm_walk *walk)
{
	pte_t huge_pte = huge_ptep_get(pte);
	struct numa_maps *md;
	struct page *page;

	if (!pte_present(huge_pte))
		return 0;

	page = pte_page(huge_pte);
	if (!page)
		return 0;

	md = walk->private;
	gather_stats(page, md, pte_dirty(huge_pte), 1);
	return 0;
}


// Source: task_mmu.c
// Lines 1774-1791
