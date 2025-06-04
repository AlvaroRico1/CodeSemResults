void *dma_direct_alloc_pages(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp, unsigned long attrs)
{
	struct page *page;
	void *ret;

	page = __dma_direct_alloc_pages(dev, size, dma_handle, gfp, attrs);
	if (!page)
		return NULL;

	if ((attrs & DMA_ATTR_NO_KERNEL_MAPPING) &&
	    !force_dma_unencrypted(dev)) {
		/* remove any dirty cache lines on the kernel alias */
		if (!PageHighMem(page))
			arch_dma_prep_coherent(page, size);
		*dma_handle = phys_to_dma(dev, page_to_phys(page));
		/* return the page pointer as the opaque cookie */
		return page;
	}

	if (PageHighMem(page)) {
		/*
		 * Depending on the cma= arguments and per-arch setup
		 * dma_alloc_contiguous could return highmem pages.
		 * Without remapping there is no way to return them here,
		 * so log an error and fail.
		 */
		dev_info(dev, "Rejecting highmem page from CMA.\n");
		__dma_direct_free_pages(dev, size, page);
		return NULL;
	}

	ret = page_address(page);
	if (force_dma_unencrypted(dev)) {
		set_memory_decrypted((unsigned long)ret, 1 << get_order(size));
		*dma_handle = __phys_to_dma(dev, page_to_phys(page));
	} else {
		*dma_handle = phys_to_dma(dev, page_to_phys(page));
	}
	memset(ret, 0, size);

	if (IS_ENABLED(CONFIG_ARCH_HAS_UNCACHED_SEGMENT) &&
	    dma_alloc_need_uncached(dev, attrs)) {
		arch_dma_prep_coherent(page, size);
		ret = uncached_kernel_address(ret);
	}

	return ret;
}


// Source: direct.c
// Lines 128-176
