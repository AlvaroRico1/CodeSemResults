int git_rebase_abort(git_rebase *rebase)
{
	git_reference *orig_head_ref = NULL;
	git_commit *orig_head_commit = NULL;
	int error;

	GIT_ASSERT_ARG(rebase);

	if (rebase->inmemory)
		return 0;

	error = rebase->head_detached ?
		git_reference_create(&orig_head_ref, rebase->repo, GIT_HEAD_FILE,
			 &rebase->orig_head_id, 1, "rebase: aborting") :
		git_reference_symbolic_create(
			&orig_head_ref, rebase->repo, GIT_HEAD_FILE, rebase->orig_head_name, 1,
			"rebase: aborting");

	if (error < 0)
		goto done;

	if ((error = git_commit_lookup(
			&orig_head_commit, rebase->repo, &rebase->orig_head_id)) < 0 ||
		(error = git_reset(rebase->repo, (git_object *)orig_head_commit,
			GIT_RESET_HARD, &rebase->options.checkout_options)) < 0)
		goto done;

	error = rebase_cleanup(rebase);

done:
	git_commit_free(orig_head_commit);
	git_reference_free(orig_head_ref);

	return error;
}


// Source: rebase.c
// Lines 1184-1218
