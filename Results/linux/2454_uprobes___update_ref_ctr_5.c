__update_ref_ctr(struct mm_struct *mm, unsigned long vaddr, short d)
{
	void *kaddr;
	struct page *page;
	struct vm_area_struct *vma;
	int ret;
	short *ptr;

	if (!vaddr || !d)
		return -EINVAL;

	ret = get_user_pages_remote(NULL, mm, vaddr, 1,
			FOLL_WRITE, &page, &vma, NULL);
	if (unlikely(ret <= 0)) {
		/*
		 * We are asking for 1 page. If get_user_pages_remote() fails,
		 * it may return 0, in that case we have to return error.
		 */
		return ret == 0 ? -EBUSY : ret;
	}

	kaddr = kmap_atomic(page);
	ptr = kaddr + (vaddr & ~PAGE_MASK);

	if (unlikely(*ptr + d < 0)) {
		pr_warn("ref_ctr going negative. vaddr: 0x%lx, "
			"curr val: %d, delta: %d\n", vaddr, *ptr, d);
		ret = -EINVAL;
		goto out;
	}

	*ptr += d;
	ret = 0;
out:
	kunmap_atomic(kaddr);
	put_page(page);
	return ret;
}


// Source: uprobes.c
// Lines 365-402
