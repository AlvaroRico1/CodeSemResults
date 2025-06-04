void git__tsort_r(
	void **dst, size_t size, git__sort_r_cmp cmp, void *payload)
{
	struct tsort_store _store, *store = &_store;
	struct tsort_run run_stack[128];

	ssize_t stack_curr = 0;
	ssize_t len, run;
	ssize_t curr = 0;
	ssize_t minrun;

	if (size < 64) {
		bisort(dst, 1, size, cmp, payload);
		return;
	}

	/* compute the minimum run length */
	minrun = (ssize_t)compute_minrun(size);

	/* temporary storage for merges */
	store->alloc = 0;
	store->storage = NULL;
	store->cmp = cmp;
	store->payload = payload;

	PUSH_NEXT();
	PUSH_NEXT();
	PUSH_NEXT();

	while (1) {
		if (!check_invariant(run_stack, stack_curr)) {
			stack_curr = collapse(dst, run_stack, stack_curr, store, size);
			continue;
		}

		PUSH_NEXT();
	}
}


// Source: tsort.c
// Lines 335-372
