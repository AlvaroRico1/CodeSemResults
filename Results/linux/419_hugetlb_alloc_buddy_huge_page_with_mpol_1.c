struct page *alloc_buddy_huge_page_with_mpol(struct hstate *h,
		struct vm_area_struct *vma, unsigned long addr)
{
	struct page *page;
	struct mempolicy *mpol;
	gfp_t gfp_mask = htlb_alloc_mask(h);
	int nid;
	nodemask_t *nodemask;

	nid = huge_node(vma, addr, gfp_mask, &mpol, &nodemask);
	page = alloc_surplus_huge_page(h, gfp_mask, nid, nodemask);
	mpol_cond_put(mpol);

	return page;
}


// Source: hugetlb.c
// Lines 1657-1671
