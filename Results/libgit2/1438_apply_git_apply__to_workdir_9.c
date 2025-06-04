static int git_apply__to_workdir(
	git_repository *repo,
	git_diff *diff,
	git_index *preimage,
	git_index *postimage,
	git_apply_location_t location,
	git_apply_options *opts)
{
	git_vector paths = GIT_VECTOR_INIT;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	const git_diff_delta *delta;
	size_t i;
	int error;

	GIT_UNUSED(opts);

	/*
	 * Limit checkout to the paths affected by the diff; this ensures
	 * that other modifications in the working directory are unaffected.
	 */
	if ((error = git_vector_init(&paths, git_diff_num_deltas(diff), NULL)) < 0)
		goto done;

	for (i = 0; i < git_diff_num_deltas(diff); i++) {
		delta = git_diff_get_delta(diff, i);

		if ((error = git_vector_insert(&paths, (void *)delta->old_file.path)) < 0)
			goto done;

		if (strcmp(delta->old_file.path, delta->new_file.path) &&
		    (error = git_vector_insert(&paths, (void *)delta->new_file.path)) < 0)
			goto done;
	}

	checkout_opts.checkout_strategy |= GIT_CHECKOUT_SAFE;
	checkout_opts.checkout_strategy |= GIT_CHECKOUT_DISABLE_PATHSPEC_MATCH;
	checkout_opts.checkout_strategy |= GIT_CHECKOUT_DONT_WRITE_INDEX;

	if (location == GIT_APPLY_LOCATION_WORKDIR)
		checkout_opts.checkout_strategy |= GIT_CHECKOUT_DONT_UPDATE_INDEX;

	checkout_opts.paths.strings = (char **)paths.contents;
	checkout_opts.paths.count = paths.length;

	checkout_opts.baseline_index = preimage;

	error = git_checkout_index(repo, postimage, &checkout_opts);

done:
	git_vector_free(&paths);
	return error;
}


// Source: apply.c
// Lines 683-734
