void test_diff_diffiter__iterate_randomly_while_saving_state(void)
{
	git_repository *repo = cl_git_sandbox_init("status");
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff *diff = NULL;
	diff_expects exp = {0};
	git_patch *patches[PATCH_CACHE];
	size_t p, d, num_d;

	memset(patches, 0, sizeof(patches));

	opts.context_lines = 3;
	opts.interhunk_lines = 1;
	opts.flags |= GIT_DIFF_INCLUDE_IGNORED | GIT_DIFF_INCLUDE_UNTRACKED;

	cl_git_pass(git_diff_index_to_workdir(&diff, repo, NULL, &opts));

	num_d = git_diff_num_deltas(diff);

	/* To make sure that references counts work for diff and patch objects,
	 * this generates patches and randomly caches them.  Only when the patch
	 * is removed from the cache are hunks and lines counted.  At the end,
	 * there are still patches in the cache, so free the diff and try to
	 * process remaining patches after the diff is freed.
	 */

	srand(121212);
	p = rand() % PATCH_CACHE;

	for (d = 0; d < num_d; ++d) {
		/* take old patch */
		git_patch *patch = patches[p];
		patches[p] = NULL;

		/* cache new patch */
		cl_git_pass(git_patch_from_diff(&patches[p], diff, d));
		cl_assert(patches[p] != NULL);

		/* process old patch if non-NULL */
		if (patch != NULL) {
			iterate_over_patch(patch, &exp);
			git_patch_free(patch);
		}

		p = rand() % PATCH_CACHE;
	}

	/* free diff list now - refcounts should keep things safe */
	git_diff_free(diff);

	/* process remaining unprocessed patches */
	for (p = 0; p < PATCH_CACHE; p++) {
		git_patch *patch = patches[p];

		if (patch != NULL) {
			iterate_over_patch(patch, &exp);
			git_patch_free(patch);
		}
	}


// Source: diffiter.c
// Lines 268-326
