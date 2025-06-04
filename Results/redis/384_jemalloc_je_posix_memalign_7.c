je_posix_memalign(void **memptr, size_t alignment, size_t size) {
	int ret;
	static_opts_t sopts;
	dynamic_opts_t dopts;

	LOG("core.posix_memalign.entry", "mem ptr: %p, alignment: %zu, "
	    "size: %zu", memptr, alignment, size);

	static_opts_init(&sopts);
	dynamic_opts_init(&dopts);

	sopts.bump_empty_alloc = true;
	sopts.min_alignment = sizeof(void *);
	sopts.oom_string =
	    "<jemalloc>: Error allocating aligned memory: out of memory\n";
	sopts.invalid_alignment_string =
	    "<jemalloc>: Error allocating aligned memory: invalid alignment\n";

	dopts.result = memptr;
	dopts.num_items = 1;
	dopts.item_size = size;
	dopts.alignment = alignment;

	ret = imalloc(&sopts, &dopts);

	LOG("core.posix_memalign.exit", "result: %d, alloc ptr: %p", ret,
	    *memptr);

	return ret;
}


// Source: jemalloc.c
// Lines 2047-2076
