void test_worktree_merge__merge_head(void)
{
	git_reference *theirs_ref, *ref;
	git_annotated_commit *theirs;

	cl_git_pass(git_reference_lookup(&theirs_ref, fixture.worktree, CONFLICT_BRANCH));
	cl_git_pass(git_annotated_commit_from_ref(&theirs, fixture.worktree, theirs_ref));
	cl_git_pass(git_merge(fixture.worktree, (const git_annotated_commit **)&theirs, 1, NULL, NULL));

	cl_git_pass(git_reference_lookup(&ref, fixture.worktree, GIT_MERGE_HEAD_FILE));

	git_reference_free(ref);
	git_reference_free(theirs_ref);
	git_annotated_commit_free(theirs);
}


// Source: merge.c
// Lines 40-54
