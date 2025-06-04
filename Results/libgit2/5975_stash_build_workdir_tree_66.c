static int build_workdir_tree(
	git_tree **tree_out,
	git_repository *repo,
	git_index *i_index,
	git_commit *b_commit)
{
	git_tree *b_tree = NULL;
	git_diff *diff = NULL, *idx_to_wd = NULL;
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	struct stash_update_rules data = {0};
	int error;

	opts.flags = GIT_DIFF_IGNORE_SUBMODULES | GIT_DIFF_INCLUDE_UNTRACKED;

	if ((error = git_commit_tree(&b_tree, b_commit)) < 0)
		goto cleanup;

	if ((error = git_diff_tree_to_index(&diff, repo, b_tree, i_index, &opts)) < 0 ||
		(error = git_diff_index_to_workdir(&idx_to_wd, repo, i_index, &opts)) < 0 ||
		(error = git_diff__merge(diff, idx_to_wd, stash_delta_merge)) < 0)
		goto cleanup;

	data.include_changed = true;

	if ((error = stash_update_index_from_diff(repo, i_index, diff, &data)) < 0)
		goto cleanup;

	error = build_tree_from_index(tree_out, repo, i_index);

cleanup:
	git_diff_free(idx_to_wd);
	git_diff_free(diff);
	git_tree_free(b_tree);

	return error;
}


// Source: stash.c
// Lines 354-389
