JEMALLOC_ATTR(malloc)
je_valloc(size_t size) {
	void *ret;

	static_opts_t sopts;
	dynamic_opts_t dopts;

	LOG("core.valloc.entry", "size: %zu\n", size);

	static_opts_init(&sopts);
	dynamic_opts_init(&dopts);

	sopts.bump_empty_alloc = true;
	sopts.null_out_result_on_error = true;
	sopts.min_alignment = PAGE;
	sopts.oom_string =
	    "<jemalloc>: Error allocating aligned memory: out of memory\n";
	sopts.invalid_alignment_string =
	    "<jemalloc>: Error allocating aligned memory: invalid alignment\n";

	dopts.result = &ret;
	dopts.num_items = 1;
	dopts.item_size = size;
	dopts.alignment = PAGE;

	imalloc(&sopts, &dopts);

	LOG("core.valloc.exit", "result: %p\n", ret);
	return ret;
}


// Source: jemalloc.c
// Lines 2453-2482
