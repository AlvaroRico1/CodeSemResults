static int commit_index(
	git_commit **i_commit,
	git_repository *repo,
	git_index *index,
	const git_signature *stasher,
	const char *message,
	const git_commit *parent)
{
	git_tree *i_tree = NULL;
	git_oid i_commit_oid;
	git_str msg = GIT_STR_INIT;
	int error;

	if ((error = build_tree_from_index(&i_tree, repo, index)) < 0)
		goto cleanup;

	if ((error = git_str_printf(&msg, "index on %s\n", message)) < 0)
		goto cleanup;

	if ((error = git_commit_create(
		&i_commit_oid,
		git_index_owner(index),
		NULL,
		stasher,
		stasher,
		NULL,
		git_str_cstr(&msg),
		i_tree,
		1,
		&parent)) < 0)
		goto cleanup;

	error = git_commit_lookup(i_commit, git_index_owner(index), &i_commit_oid);

cleanup:
	git_tree_free(i_tree);
	git_str_dispose(&msg);
	return error;
}


// Source: stash.c
// Lines 120-158
