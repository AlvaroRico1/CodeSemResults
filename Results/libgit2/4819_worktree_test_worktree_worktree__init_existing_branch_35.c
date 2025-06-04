void test_worktree_worktree__init_existing_branch(void)
{
	git_reference *head, *branch;
	git_commit *commit;
	git_worktree *wt;
	git_str path = GIT_STR_INIT;

	cl_git_pass(git_repository_head(&head, fixture.repo));
	cl_git_pass(git_commit_lookup(&commit, fixture.repo, &head->target.oid));
	cl_git_pass(git_branch_create(&branch, fixture.repo, "worktree-new", commit, false));

	cl_git_pass(git_str_joinpath(&path, fixture.repo->workdir, "../worktree-new"));
	cl_git_fail(git_worktree_add(&wt, fixture.repo, "worktree-new", path.ptr, NULL));

	git_str_dispose(&path);
	git_commit_free(commit);
	git_reference_free(head);
	git_reference_free(branch);
}


// Source: worktree.c
// Lines 245-263
