void test_worktree_merge__merge_conflict(void)
{
	git_str path = GIT_STR_INIT, buf = GIT_STR_INIT;
	git_reference *theirs_ref;
	git_annotated_commit *theirs;
	git_index *index;
	const git_index_entry *entry;
	size_t i, conflicts = 0;

	cl_git_pass(git_reference_lookup(&theirs_ref, fixture.worktree, CONFLICT_BRANCH));
	cl_git_pass(git_annotated_commit_from_ref(&theirs, fixture.worktree, theirs_ref));

	cl_git_pass(git_merge(fixture.worktree,
		    (const git_annotated_commit **)&theirs, 1, NULL, NULL));

	cl_git_pass(git_repository_index(&index, fixture.worktree));
	for (i = 0; i < git_index_entrycount(index); i++) {
		cl_assert(entry = git_index_get_byindex(index, i));

		if (git_index_entry_is_conflict(entry))
			conflicts++;
	}
	cl_assert_equal_sz(conflicts, 3);

	git_reference_free(theirs_ref);
	git_annotated_commit_free(theirs);
	git_index_free(index);

	cl_git_pass(git_str_joinpath(&path, fixture.worktree->workdir, "branch_file.txt"));
	cl_git_pass(git_futils_readbuffer(&buf, path.ptr));
	cl_assert_equal_s(buf.ptr, CONFLICT_BRANCH_FILE_TXT);

	git_str_dispose(&path);
	git_str_dispose(&buf);
}


// Source: merge.c
// Lines 86-120
