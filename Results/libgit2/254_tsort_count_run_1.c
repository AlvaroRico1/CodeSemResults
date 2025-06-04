static ssize_t count_run(
	void **dst, ssize_t start, ssize_t size, struct tsort_store *store)
{
	ssize_t curr = start + 2;

	if (size - start == 1)
		return 1;

	if (start >= size - 2) {
		if (store->cmp(dst[size - 2], dst[size - 1], store->payload) > 0) {
			void *tmp = dst[size - 1];
			dst[size - 1] = dst[size - 2];
			dst[size - 2] = tmp;
		}


// Source: tsort.c
// Lines 120-133
