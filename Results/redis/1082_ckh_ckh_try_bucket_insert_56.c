ckh_try_bucket_insert(ckh_t *ckh, size_t bucket, const void *key,
    const void *data) {
	ckhc_t *cell;
	unsigned offset, i;

	/*
	 * Cycle through the cells in the bucket, starting at a random position.
	 * The randomness avoids worst-case search overhead as buckets fill up.
	 */
	offset = (unsigned)prng_lg_range_u64(&ckh->prng_state,
	    LG_CKH_BUCKET_CELLS);
	for (i = 0; i < (ZU(1) << LG_CKH_BUCKET_CELLS); i++) {
		cell = &ckh->tab[(bucket << LG_CKH_BUCKET_CELLS) +
		    ((i + offset) & ((ZU(1) << LG_CKH_BUCKET_CELLS) - 1))];
		if (cell->key == NULL) {
			cell->key = key;
			cell->data = data;
			ckh->count++;
			return false;
		}
	}

	return true;
}


// Source: ckh.c
// Lines 102-125
