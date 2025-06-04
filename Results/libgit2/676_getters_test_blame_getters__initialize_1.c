void test_blame_getters__initialize(void)
{
	size_t i;
	git_blame_options opts = GIT_BLAME_OPTIONS_INIT;

	git_blame_hunk hunks[] = {
		{ 3, {{0}},  1, NULL, {{0}}, "a", 0},
		{ 3, {{0}},  4, NULL, {{0}}, "b", 0},
		{ 3, {{0}},  7, NULL, {{0}}, "c", 0},
		{ 3, {{0}}, 10, NULL, {{0}}, "d", 0},
		{ 3, {{0}}, 13, NULL, {{0}}, "e", 0},
	};

	g_blame = git_blame__alloc(NULL, opts, "");

	for (i=0; i<5; i++) {
		git_blame_hunk *h = git__calloc(1, sizeof(git_blame_hunk));
		h->final_start_line_number = hunks[i].final_start_line_number;
		h->orig_path = git__strdup(hunks[i].orig_path);
		h->lines_in_hunk = hunks[i].lines_in_hunk;

		git_vector_insert(&g_blame->hunks, h);
	}


// Source: getters.c
// Lines 7-29
