static int merge_bases(git_commit_list **out, git_revwalk **walk_out, git_repository *repo, const git_oid *one, const git_oid *two)
{
	git_revwalk *walk;
	git_vector list;
	git_commit_list *result = NULL;
	git_commit_list_node *commit;
	void *contents[1];

	if (git_revwalk_new(&walk, repo) < 0)
		return -1;

	commit = git_revwalk__commit_lookup(walk, two);
	if (commit == NULL)
		goto on_error;

	/* This is just one value, so we can do it on the stack */
	memset(&list, 0x0, sizeof(git_vector));
	contents[0] = commit;
	list.length = 1;
	list.contents = contents;

	commit = git_revwalk__commit_lookup(walk, one);
	if (commit == NULL)
		goto on_error;

	if (git_merge__bases_many(&result, walk, commit, &list, 0) < 0)
		goto on_error;

	if (!result) {
		git_revwalk_free(walk);
		git_error_set(GIT_ERROR_MERGE, "no merge base found");
		return GIT_ENOTFOUND;
	}

	*out = result;
	*walk_out = walk;

	return 0;

on_error:
	git_revwalk_free(walk);
	return -1;

}


// Source: merge.c
// Lines 221-264
