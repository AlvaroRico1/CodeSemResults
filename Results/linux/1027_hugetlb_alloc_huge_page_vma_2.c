struct page *alloc_huge_page_vma(struct hstate *h, struct vm_area_struct *vma,
		unsigned long address)
{
	struct mempolicy *mpol;
	nodemask_t *nodemask;
	struct page *page;
	gfp_t gfp_mask;
	int node;

	gfp_mask = htlb_alloc_mask(h);
	node = huge_node(vma, address, gfp_mask, &mpol, &nodemask);
	page = alloc_huge_page_nodemask(h, node, nodemask);
	mpol_cond_put(mpol);

	return page;
}


// Source: hugetlb.c
// Lines 1715-1730
