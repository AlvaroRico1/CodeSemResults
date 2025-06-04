static int resize(struct tsort_store *store, size_t new_size)
{
	if (store->alloc < new_size) {
		void **tempstore;

		tempstore = git__reallocarray(store->storage, new_size, sizeof(void *));

		/**
		 * Do not propagate on OOM; this will abort the sort and
		 * leave the array unsorted, but no error code will be
		 * raised
		 */
		if (tempstore == NULL)
			return -1;

		store->storage = tempstore;
		store->alloc = new_size;
	}


// Source: tsort.c
// Lines 182-199
