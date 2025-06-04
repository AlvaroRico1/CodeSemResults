JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE2(1, 2)
je_calloc(size_t num, size_t size) {
	void *ret;
	static_opts_t sopts;
	dynamic_opts_t dopts;

	LOG("core.calloc.entry", "num: %zu, size: %zu\n", num, size);

	static_opts_init(&sopts);
	dynamic_opts_init(&dopts);

	sopts.may_overflow = true;
	sopts.bump_empty_alloc = true;
	sopts.null_out_result_on_error = true;
	sopts.set_errno_on_error = true;
	sopts.oom_string = "<jemalloc>: Error in calloc(): out of memory\n";

	dopts.result = &ret;
	dopts.num_items = num;
	dopts.item_size = size;
	dopts.zero = true;

	imalloc(&sopts, &dopts);

	LOG("core.calloc.exit", "result: %p", ret);

	return ret;
}


// Source: jemalloc.c
// Lines 2116-2143
