static int branch_is_checked_out(git_repository *worktree, void *payload)
{
	git_reference *branch = (git_reference *) payload;
	git_reference *head = NULL;
	int error;

	if (git_repository_is_bare(worktree))
		return 0;

	if ((error = git_reference_lookup(&head, worktree, GIT_HEAD_FILE)) < 0) {
		if (error == GIT_ENOTFOUND)
			error = 0;
		goto out;
	}

	if (git_reference_type(head) != GIT_REFERENCE_SYMBOLIC)
		goto out;

	error = !git__strcmp(head->target.symbolic, branch->name);

out:
	git_reference_free(head);
	return error;
}


// Source: branch.c
// Lines 143-166
