base_delete(tsdn_t *tsdn, base_t *base) {
	extent_hooks_t *extent_hooks = base_extent_hooks_get(base);
	base_block_t *next = base->blocks;
	do {
		base_block_t *block = next;
		next = block->next;
		base_unmap(tsdn, extent_hooks, base_ind_get(base), block,
		    block->size);
	} while (next != NULL);


// Source: base.c
// Lines 396-404
