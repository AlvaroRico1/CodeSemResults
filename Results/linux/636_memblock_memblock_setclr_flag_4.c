static int __init_memblock memblock_setclr_flag(phys_addr_t base,
				phys_addr_t size, int set, int flag)
{
	struct memblock_type *type = &memblock.memory;
	int i, ret, start_rgn, end_rgn;

	ret = memblock_isolate_range(type, base, size, &start_rgn, &end_rgn);
	if (ret)
		return ret;

	for (i = start_rgn; i < end_rgn; i++) {
		struct memblock_region *r = &type->regions[i];

		if (set)
			r->flags |= flag;
		else
			r->flags &= ~flag;
	}

	memblock_merge_regions(type);
	return 0;
}


// Source: memblock.c
// Lines 848-869
