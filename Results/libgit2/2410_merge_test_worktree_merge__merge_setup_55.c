void test_worktree_merge__merge_setup(void)
{
	git_reference *ours_ref, *theirs_ref;
	git_annotated_commit *ours, *theirs;
	git_str path = GIT_STR_INIT;
	unsigned i;

	cl_git_pass(git_reference_lookup(&ours_ref, fixture.worktree, MASTER_BRANCH));
	cl_git_pass(git_annotated_commit_from_ref(&ours, fixture.worktree, ours_ref));

	cl_git_pass(git_reference_lookup(&theirs_ref, fixture.worktree, CONFLICT_BRANCH));
	cl_git_pass(git_annotated_commit_from_ref(&theirs, fixture.worktree, theirs_ref));

	cl_git_pass(git_merge__setup(fixture.worktree,
		    ours, (const git_annotated_commit **)&theirs, 1));

	for (i = 0; i < ARRAY_SIZE(merge_files); i++) {
		cl_git_pass(git_str_joinpath(&path,
		            fixture.worktree->gitdir,
		            merge_files[i]));
		cl_assert(git_fs_path_exists(path.ptr));
	}

	git_str_dispose(&path);
	git_reference_free(ours_ref);
	git_reference_free(theirs_ref);
	git_annotated_commit_free(ours);
	git_annotated_commit_free(theirs);
}


// Source: merge.c
// Lines 56-84
