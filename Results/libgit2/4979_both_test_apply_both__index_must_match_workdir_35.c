void test_apply_both__index_must_match_workdir(void)
{
	git_diff *diff;
	git_index *index;
	git_index_entry idx_entry;

	const char *diff_file = DIFF_MODIFY_TWO_FILES;

	/*
	 * Append a line to the end of the file in both the index and the
	 * working directory.  Although the appended line would allow for
	 * patch application in each, the line appended is different in
	 * each, so the application should not be allowed.
	 */
	cl_git_append2file("merge-recursive/asparagus.txt",
	    "This is a modification.\n");

	cl_git_pass(git_repository_index(&index, repo));

	memset(&idx_entry, 0, sizeof(git_index_entry));
	idx_entry.mode = 0100644;
	idx_entry.path = "asparagus.txt";
	cl_git_pass(git_oid_fromstr(&idx_entry.id, "06d3fefb8726ab1099acc76e02dfb85e034b2538"));
	cl_git_pass(git_index_add(index, &idx_entry));

	cl_git_pass(git_index_write(index));
	git_index_free(index);

	cl_git_pass(git_diff_from_buffer(&diff, diff_file, strlen(diff_file)));
	cl_git_fail_with(GIT_EAPPLYFAIL, git_apply(repo, diff, GIT_APPLY_LOCATION_BOTH, NULL));

	git_diff_free(diff);
}


// Source: both.c
// Lines 173-205
