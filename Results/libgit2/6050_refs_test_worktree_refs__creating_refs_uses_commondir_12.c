void test_worktree_refs__creating_refs_uses_commondir(void)
{
	   git_reference *head, *branch, *lookup;
	   git_commit *commit;
	   git_str refpath = GIT_STR_INIT;

	   cl_git_pass(git_str_joinpath(&refpath,
		       git_repository_commondir(fixture.worktree), "refs/heads/testbranch"));
	   cl_assert(!git_fs_path_exists(refpath.ptr));

	   cl_git_pass(git_repository_head(&head, fixture.worktree));
	   cl_git_pass(git_commit_lookup(&commit, fixture.worktree, git_reference_target(head)));
	   cl_git_pass(git_branch_create(&branch, fixture.worktree, "testbranch", commit, 0));
	   cl_git_pass(git_branch_lookup(&lookup, fixture.worktree, "testbranch", GIT_BRANCH_LOCAL));
	   cl_assert(git_reference_cmp(branch, lookup) == 0);
	   cl_assert(git_fs_path_exists(refpath.ptr));

	   git_reference_free(lookup);
	   git_reference_free(branch);
	   git_reference_free(head);
	   git_commit_free(commit);
	   git_str_dispose(&refpath);
}


// Source: refs.c
// Lines 176-198
