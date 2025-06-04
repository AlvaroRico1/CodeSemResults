static void bisort(
	void **dst, size_t start, size_t size, git__sort_r_cmp cmp, void *payload)
{
	size_t i;
	void *x;
	int location;

	for (i = start; i < size; i++) {
		int j;
		/* If this entry is already correct, just move along */
		if (cmp(dst[i - 1], dst[i], payload) <= 0)
			continue;

		/* Else we need to find the right place, shift everything over, and squeeze in */
		x = dst[i];
		location = binsearch(dst, x, i, cmp, payload);
		for (j = (int)i - 1; j >= location; j--) {
			dst[j + 1] = dst[j];
		}
		dst[location] = x;
	}
}


// Source: tsort.c
// Lines 71-92
