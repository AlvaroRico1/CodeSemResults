JEMALLOC_ATTR(malloc) JEMALLOC_ALLOC_SIZE(1)
je_malloc(size_t size) {
	void *ret;
	static_opts_t sopts;
	dynamic_opts_t dopts;

	LOG("core.malloc.entry", "size: %zu", size);

	static_opts_init(&sopts);
	dynamic_opts_init(&dopts);

	sopts.bump_empty_alloc = true;
	sopts.null_out_result_on_error = true;
	sopts.set_errno_on_error = true;
	sopts.oom_string = "<jemalloc>: Error in malloc(): out of memory\n";

	dopts.result = &ret;
	dopts.num_items = 1;
	dopts.item_size = size;

	imalloc(&sopts, &dopts);

	LOG("core.malloc.exit", "result: %p", ret);

	return ret;
}


// Source: jemalloc.c
// Lines 2018-2043
