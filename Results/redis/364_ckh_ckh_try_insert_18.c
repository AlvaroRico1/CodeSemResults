ckh_try_insert(ckh_t *ckh, void const**argkey, void const**argdata) {
	size_t hashes[2], bucket;
	const void *key = *argkey;
	const void *data = *argdata;

	ckh->hash(key, hashes);

	/* Try to insert in primary bucket. */
	bucket = hashes[0] & ((ZU(1) << ckh->lg_curbuckets) - 1);
	if (!ckh_try_bucket_insert(ckh, bucket, key, data)) {
		return false;
	}

	/* Try to insert in secondary bucket. */
	bucket = hashes[1] & ((ZU(1) << ckh->lg_curbuckets) - 1);
	if (!ckh_try_bucket_insert(ckh, bucket, key, data)) {
		return false;
	}

	/*
	 * Try to find a place for this item via iterative eviction/relocation.
	 */
	return ckh_evict_reloc_insert(ckh, bucket, argkey, argdata);
}


// Source: ckh.c
// Lines 205-228
