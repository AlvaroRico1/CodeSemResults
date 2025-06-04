int git_diff__merge(
	git_diff *onto, const git_diff *from, git_diff__merge_cb cb)
{
	int error = 0;
	git_pool onto_pool;
	git_vector onto_new;
	git_diff_delta *delta;
	bool ignore_case, reversed;
	unsigned int i, j;

	GIT_ASSERT_ARG(onto);
	GIT_ASSERT_ARG(from);

	if (!from->deltas.length)
		return 0;

	ignore_case = ((onto->opts.flags & GIT_DIFF_IGNORE_CASE) != 0);
	reversed    = ((onto->opts.flags & GIT_DIFF_REVERSE) != 0);

	if (ignore_case != ((from->opts.flags & GIT_DIFF_IGNORE_CASE) != 0) ||
		reversed    != ((from->opts.flags & GIT_DIFF_REVERSE) != 0)) {
		git_error_set(GIT_ERROR_INVALID,
			"attempt to merge diffs created with conflicting options");
		return -1;
	}

	if (git_vector_init(&onto_new, onto->deltas.length, git_diff_delta__cmp) < 0 ||
	    git_pool_init(&onto_pool, 1) < 0)
		return -1;

	for (i = 0, j = 0; i < onto->deltas.length || j < from->deltas.length; ) {
		git_diff_delta *o = GIT_VECTOR_GET(&onto->deltas, i);
		const git_diff_delta *f = GIT_VECTOR_GET(&from->deltas, j);
		int cmp = !f ? -1 : !o ? 1 :
			STRCMP_CASESELECT(ignore_case, o->old_file.path, f->old_file.path);

		if (cmp < 0) {
			delta = git_diff__delta_dup(o, &onto_pool);
			i++;
		} else if (cmp > 0) {
			delta = git_diff__delta_dup(f, &onto_pool);
			j++;
		} else {
			const git_diff_delta *left = reversed ? f : o;
			const git_diff_delta *right = reversed ? o : f;

			delta = cb(left, right, &onto_pool);
			i++;
			j++;
		}


// Source: diff_tform.c
// Lines 114-163
