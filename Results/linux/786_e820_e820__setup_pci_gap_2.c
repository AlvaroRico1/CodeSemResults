__init void e820__setup_pci_gap(void)
{
	unsigned long gapstart, gapsize;
	int found;

	gapsize = 0x400000;
	found  = e820_search_gap(&gapstart, &gapsize);

	if (!found) {
#ifdef CONFIG_X86_64
		gapstart = (max_pfn << PAGE_SHIFT) + 1024*1024;
		pr_err("Cannot find an available gap in the 32-bit address range\n");
		pr_err("PCI devices with unassigned 32-bit BARs may not work!\n");
#else
		gapstart = 0x10000000;
#endif
	}

	/*
	 * e820__reserve_resources_late() protects stolen RAM already:
	 */
	pci_mem_start = gapstart;

	pr_info("[mem %#010lx-%#010lx] available for PCI devices\n",
		gapstart, gapstart + gapsize - 1);
}


// Source: e820.c
// Lines 643-668
