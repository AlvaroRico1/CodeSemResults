static int rebase_init_operations(
	git_rebase *rebase,
	git_repository *repo,
	const git_annotated_commit *branch,
	const git_annotated_commit *upstream,
	const git_annotated_commit *onto)
{
	git_revwalk *revwalk = NULL;
	git_commit *commit;
	git_oid id;
	bool merge;
	git_rebase_operation *operation;
	int error;

	if (!upstream)
		upstream = onto;

	if ((error = git_revwalk_new(&revwalk, rebase->repo)) < 0 ||
		(error = git_revwalk_push(revwalk, git_annotated_commit_id(branch))) < 0 ||
		(error = git_revwalk_hide(revwalk, git_annotated_commit_id(upstream))) < 0)
		goto done;

	git_revwalk_sorting(revwalk, GIT_SORT_REVERSE);

	while ((error = git_revwalk_next(&id, revwalk)) == 0) {
		if ((error = git_commit_lookup(&commit, repo, &id)) < 0)
			goto done;

		merge = (git_commit_parentcount(commit) > 1);
		git_commit_free(commit);

		if (merge)
			continue;

		operation = rebase_operation_alloc(rebase, GIT_REBASE_OPERATION_PICK, &id, NULL);
		GIT_ERROR_CHECK_ALLOC(operation);
	}

	error = 0;

done:
	git_revwalk_free(revwalk);
	return error;
}


// Source: rebase.c
// Lines 576-619
