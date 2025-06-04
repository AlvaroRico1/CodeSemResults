struct bitmap_commit_cb {
	struct commit **commits;
	size_t commits_nr, commits_alloc;

	struct write_midx_context *ctx;
};


// Source: midx.c
// Lines 924-929
