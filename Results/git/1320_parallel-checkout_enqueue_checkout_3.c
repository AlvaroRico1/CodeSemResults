int enqueue_checkout(struct cache_entry *ce, struct conv_attrs *ca)
{
	struct parallel_checkout_item *pc_item;

	if (parallel_checkout.status != PC_ACCEPTING_ENTRIES ||
	    !is_eligible_for_parallel_checkout(ce, ca))
		return -1;

	ALLOC_GROW(parallel_checkout.items, parallel_checkout.nr + 1,
		   parallel_checkout.alloc);

	pc_item = &parallel_checkout.items[parallel_checkout.nr];
	pc_item->ce = ce;
	memcpy(&pc_item->ca, ca, sizeof(pc_item->ca));
	pc_item->status = PC_ITEM_PENDING;
	pc_item->id = parallel_checkout.nr;
	parallel_checkout.nr++;

	return 0;
}


// Source: parallel-checkout.c
// Lines 146-165
