void __init memblock_cap_memory_range(phys_addr_t base, phys_addr_t size)
{
	int start_rgn, end_rgn;
	int i, ret;

	if (!size)
		return;

	ret = memblock_isolate_range(&memblock.memory, base, size,
						&start_rgn, &end_rgn);
	if (ret)
		return;

	/* remove all the MAP regions */
	for (i = memblock.memory.cnt - 1; i >= end_rgn; i--)
		if (!memblock_is_nomap(&memblock.memory.regions[i]))
			memblock_remove_region(&memblock.memory, i);

	for (i = start_rgn - 1; i >= 0; i--)
		if (!memblock_is_nomap(&memblock.memory.regions[i]))
			memblock_remove_region(&memblock.memory, i);

	/* truncate the reserved regions */
	memblock_remove_range(&memblock.reserved, 0, base);
	memblock_remove_range(&memblock.reserved,
			base + size, PHYS_ADDR_MAX);
}


// Source: memblock.c
// Lines 1668-1694
