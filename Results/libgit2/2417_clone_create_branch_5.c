static int create_branch(
	git_reference **branch,
	git_repository *repo,
	const git_oid *target,
	const char *name,
	const char *log_message)
{
	git_commit *head_obj = NULL;
	git_reference *branch_ref = NULL;
	git_str refname = GIT_STR_INIT;
	int error;

	/* Find the target commit */
	if ((error = git_commit_lookup(&head_obj, repo, target)) < 0)
		return error;

	/* Create the new branch */
	if ((error = git_str_printf(&refname, GIT_REFS_HEADS_DIR "%s", name)) < 0)
		return error;

	error = git_reference_create(&branch_ref, repo, git_str_cstr(&refname), target, 0, log_message);
	git_str_dispose(&refname);
	git_commit_free(head_obj);

	if (!error)
		*branch = branch_ref;
	else
		git_reference_free(branch_ref);

	return error;
}


// Source: clone.c
// Lines 28-58
