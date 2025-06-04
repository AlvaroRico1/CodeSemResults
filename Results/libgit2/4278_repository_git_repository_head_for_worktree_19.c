int git_repository_head_for_worktree(git_reference **out, git_repository *repo, const char *name)
{
	git_repository *worktree_repo = NULL;
	git_worktree *worktree = NULL;
	git_reference *head = NULL;
	int error;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(name);

	*out = NULL;

	if ((error = git_worktree_lookup(&worktree, repo, name)) < 0 ||
	    (error = git_repository_open_from_worktree(&worktree_repo, worktree)) < 0 ||
	    (error = git_reference_lookup(&head, worktree_repo, GIT_HEAD_FILE)) < 0)
		goto out;

	if (git_reference_type(head) != GIT_REFERENCE_DIRECT) {
		if ((error = git_reference_lookup_resolved(out, worktree_repo, git_reference_symbolic_target(head), -1)) < 0)
			goto out;
	} else {
		*out = head;
		head = NULL;
	}

out:
	git_reference_free(head);
	git_worktree_free(worktree);
	git_repository_free(worktree_repo);
	return error;
}


// Source: repository.c
// Lines 2407-2438
